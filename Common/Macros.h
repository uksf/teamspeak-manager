#pragma once

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

#define PROCEDURE_FUNCTION(type,name) class name## : public IProcedureFunction { \
DECLARE_MEMBER(CLIENT_MESSAGE_TYPE, Type) \
public: \
    name##(){ this->m_Type = type; } \
    ~##name(){ } \
    void call(ClientMessage message)
