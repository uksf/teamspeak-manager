#include "PipeManager.h"
#include "TextMessage.h"
#include "Engine.h"
#include <wincon.h>

#define PIPE_NAME_TO "\\\\.\\pipe\\teamspeak-manager-toTS"
#define PIPE_NAME_FROM "\\\\.\\pipe\\teamspeak-manager-fromTS"

PipeManager::PipeManager() {  // NOLINT(cppcoreguidelines-pro-type-member-init)
    this->PipeManager::setConnectedWrite(FALSE);
    this->PipeManager::setConnectedRead(FALSE);
    this->PipeManager::setPipeHandleWrite(INVALID_HANDLE_VALUE);
    this->PipeManager::setPipeHandleRead(INVALID_HANDLE_VALUE);
    this->PipeManager::setShuttingDown(FALSE);
    this->PipeManager::setFromPipeName(PIPE_NAME_FROM);
    this->PipeManager::setToPipeName(PIPE_NAME_TO);
}

PipeManager::~PipeManager() {
    this->PipeManager::shutdown();
}

void PipeManager::initialize() {
    HANDLE writeHandle = nullptr, readHandle = nullptr;
    BOOL tryAgain = TRUE;

    LOG("Opening game pipe...");
    while (tryAgain) {
        writeHandle = CreateNamedPipeA(
            this->getFromPipeName().c_str(), // name of the pipe
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | // message-type pipe 
            PIPE_READMODE_MESSAGE, // send data as message
            PIPE_UNLIMITED_INSTANCES,
            4096, // no outbound buffer
            4096, // no inbound buffer
            0, // use default wait time
            nullptr // use no security attributes
        );
        if (writeHandle == INVALID_HANDLE_VALUE) {
            char errstr[1024];

            _snprintf_s(errstr, sizeof errstr, "Conflicting game write pipe detected, could not create pipe!\nERROR CODE: %d", GetLastError());
            const int ret = MessageBoxA(nullptr, errstr, "TSM Error", MB_RETRYCANCEL | MB_ICONEXCLAMATION);
            if (ret != IDRETRY) {
                tryAgain = FALSE;
                TerminateProcess(GetCurrentProcess(), 0);
            }
        } else {
            tryAgain = FALSE;
        }
    }

    tryAgain = TRUE;
    while (tryAgain) {
        readHandle = CreateNamedPipeA(
            this->getToPipeName().c_str(), // name of the pipe
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | // message-type pipe 
            PIPE_NOWAIT | // Deprecated but fuck it, it is simpler.
            PIPE_READMODE_MESSAGE, // send data as message
            PIPE_UNLIMITED_INSTANCES,
            4096, // no outbound buffer
            4096, // no inbound buffer
            0, // use default wait time
            nullptr // use no security attributes
        );
        if (readHandle == INVALID_HANDLE_VALUE) {
            char errstr[1024];

            _snprintf_s(errstr, sizeof errstr, "Conflicting game read pipe detected, could not create pipe!\nERROR CODE: %d", GetLastError());
            const int ret = MessageBoxA(nullptr, errstr, "TSM Error", MB_RETRYCANCEL | MB_ICONEXCLAMATION);
            if (ret != IDRETRY) {
                tryAgain = FALSE;
                TerminateProcess(GetCurrentProcess(), 0);
            }
        } else {
            tryAgain = FALSE;
        }
    }

    this->setPipeHandleRead(readHandle);
    this->setPipeHandleWrite(writeHandle);

    LOG("Game pipe opening successful. [%d & %d]", this->getPipeHandleRead(), this->getPipeHandleWrite());

    this->m_sendThread = std::thread(&PipeManager::sendLoop, this);
    this->m_readThread = std::thread(&PipeManager::readLoop, this);
}

void PipeManager::shutdown() {
    this->setShuttingDown(true);
    this->setConnectedWrite(FALSE);
    this->setConnectedRead(FALSE);

    HANDLE hPipe = CreateFile(LPCWSTR(this->getToPipeName().c_str()), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hPipe != INVALID_HANDLE_VALUE) {
        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
    }

    if (this->m_readThread.joinable()) {
        this->m_readThread.join();
    }

    hPipe = CreateFile(LPCWSTR(this->getFromPipeName().c_str()), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hPipe != INVALID_HANDLE_VALUE) {
        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
    }

    if (this->m_sendThread.joinable()) {
        this->m_sendThread.join();
    }

    this->setShuttingDown(false);
}

void PipeManager::sendLoop() {
    DWORD cbWritten;
    IMessage *msg;

    while (!this->getShuttingDown()) {

        do {
            ConnectNamedPipe(this->m_PipeHandleWrite, nullptr);
            if (GetLastError() == ERROR_PIPE_CONNECTED) {
                LOG("Client write connected");
                this->setConnectedWrite(TRUE);
                break;
            }
            this->setConnectedWrite(FALSE);
            Sleep(1);
        } while (!this->getConnectedWrite() && !this->getShuttingDown());

        clock_t lastTick = clock() / CLOCKS_PER_SEC;
        while (this->getConnectedWrite()) {
            if (this->getShuttingDown())
                break;

            const clock_t tick = clock() / CLOCKS_PER_SEC;
            if (tick - lastTick > PIPE_TIMEOUT / 1000) {
                LOG("No send message for %d seconds, disconnecting", (PIPE_TIMEOUT / 1000));
                this->setConnectedWrite(FALSE);
                break;
            }

            if (this->m_sendQueue.try_pop(msg)) {
                if (msg) {
                    lastTick = clock() / CLOCKS_PER_SEC;
                    const DWORD size = DWORD(strlen(reinterpret_cast<char *>(msg->getData()))) + 1;
                    if (size > 3) {
                        this->lock();
                        const BOOL ret = WriteFile(
                            this->m_PipeHandleWrite,     // pipe handle 
                            msg->getData(),                    // message 
                            size,                    // message length 
                            &cbWritten,             // bytes written 
                            nullptr);                  // not overlapped 
                        this->unlock();

                        if (!ret) {
                            LOG("WriteFile failed, [%d]", GetLastError());
                            if (GetLastError() == ERROR_BROKEN_PIPE) {
                                this->setConnectedWrite(FALSE);
                            }
                        }
                    }
                    delete msg;
                }
            }
            Sleep(1);
        }
        LOG("Write loop disconnected");
        FlushFileBuffers(this->m_PipeHandleWrite);
        DisconnectNamedPipe(this->m_PipeHandleWrite);
        Sleep(1);
    }
}

void PipeManager::readLoop() {
    DWORD cbRead;

    auto*mBuffer = static_cast<char *>(LocalAlloc(LMEM_FIXED, BUFSIZE));
    if (!mBuffer) {
        LOG("LocalAlloc() failed: %d", GetLastError());
    }
    while (!this->getShuttingDown()) {
        ConnectNamedPipe(this->m_PipeHandleRead, nullptr);
        if (GetLastError() == ERROR_PIPE_CONNECTED) {
            LOG("Client read connected");
            this->setConnectedRead(TRUE);
        } else {
            this->setConnectedRead(FALSE);
            Sleep(1);

            continue;
        }
        clock_t lastTick = clock() / CLOCKS_PER_SEC;
        while (this->getConnectedRead()) {
            if (this->getShuttingDown())
                break;

            const clock_t tick = clock() / CLOCKS_PER_SEC;
            if (tick - lastTick > PIPE_TIMEOUT / 100) {
                LOG("No read message for %d seconds, disconnecting", (PIPE_TIMEOUT / 1000));
                this->setConnectedWrite(FALSE);
                this->setConnectedRead(FALSE);
                break;
            }

            BOOL ret;
            do {
                ret = ReadFile(this->m_PipeHandleRead, mBuffer, BUFSIZE, &cbRead, nullptr);
                if (!ret && GetLastError() != ERROR_MORE_DATA) {
                    break;
                }
                if (!ret && GetLastError() == ERROR_BROKEN_PIPE) {
                    this->setConnectedRead(FALSE);
                    break;
                }
                mBuffer[cbRead] = 0x00;
                IMessage *msg = new TextMessage(static_cast<char *>(mBuffer), cbRead);
                if (msg && msg->getProcedureName()) {
                    Engine::getInstance()->getProcedureEngine()->runProcedure(this, msg);
                    lastTick = clock() / CLOCKS_PER_SEC;
                }
            } while (!ret);
            Sleep(1);
        }
        this->setConnectedWrite(FALSE);
        this->setConnectedRead(FALSE);
        FlushFileBuffers(this->m_PipeHandleRead);
        DisconnectNamedPipe(this->m_PipeHandleRead);
        LOG("Client disconnected");

        this->m_sendQueue.clear();
        Sleep(1);
    }

    if (mBuffer)
        LocalFree(mBuffer);
}

void PipeManager::sendMessage(IMessage *message) {
    if (message) {
        this->m_sendQueue.push(message);
    }
}
