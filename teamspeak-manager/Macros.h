#pragma once

#define PIPE_TIMEOUT 5000
#define BUFSIZE 10240

#define STATE_RUNNING 1
#define STATE_INITIALIZING 2
#define STATE_STOPPING 3
#define STATE_STARTING 4
#define STATE_READY 5
#define STATE_STOPPED 0xFFFFFFFF

#define STR(x) #x

#define DECLARE_MEMBER_SET(type, name) \
    virtual __inline void set##name(##type value) { this->m_##name = value; }

#define DECLARE_MEMBER_GET(type, name) \
    virtual __inline type get##name() { return this->m_##name; }

#define DECLARE_MEMBER_PRIVATE(type, name) \
    type m_##name;


#define DECLARE_MEMBER(type, name) \
public: \
    DECLARE_MEMBER_SET(type, name) \
    DECLARE_MEMBER_GET(type, name) \
protected: \
    DECLARE_MEMBER_PRIVATE(type, name)

#define DECLARE_INTERFACE_MEMBER_SET(type, name) \
    virtual void set##name(##type value) = 0;

#define DECLARE_INTERFACE_MEMBER_GET(type, name) \
    virtual type get##name() = 0;

#define DECLARE_INTERFACE_MEMBER(type, name) \
public: \
    DECLARE_INTERFACE_MEMBER_SET(type, name) \
    DECLARE_INTERFACE_MEMBER_GET(type, name)

#define LOCK(x) x->lock();
#define UNLOCK(x) x->unlock();
#define LOCK_WRITE(x) x->lock_write();
#define UNLOCK_WRITE(x) x->unlock_write();

#define PROCEDURE_FUNCTION(name) class name## : public IProcedureFunction { \
public: \
    name##(){ this->m_Name = STR(name); } \
    ~##name(){ } \
    void call(IServer *vServer, IMessage *vMessage)

