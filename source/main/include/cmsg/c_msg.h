#ifndef __CMSG_MSG_H__
#define __CMSG_MSG_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ccore/c_debug.h"
#include "ccore/c_allocator.h"
#include "cbase/c_memory.h"

namespace ncore
{
    namespace necs
    {
        struct entity_t;
        struct system_t;
    } // namespace necs

    namespace nmsg
    {
        typedef u64   id_t;
        typedef void* msg_t;
        typedef void* property_t;
        typedef void* value_t;

        struct typeinfo_t
        {
            inline typeinfo_t()
                : m_type_name(nullptr)
                , m_default_value(nullptr)
                , m_sizeof(0)
                , m_id(nullptr)
            {
            }
            inline typeinfo_t(const char* type_name, const void* default_value, u32 sizeof_type)
                : m_type_name(type_name)
                , m_default_value(default_value)
                , m_sizeof(sizeof_type)
                , m_id(nullptr)
            {
            }

            const char* m_type_name;
            const void* m_default_value;
            void*       m_id;
            u64         m_sizeof;
        };

        template <typename T> struct type_t
        {
            static const T    default_value;
            static typeinfo_t typeinfo;
        };

        // ------------------------------------------------------------------------------------------------
        // types
        static const type_t<u8>   type_u8;
        static const type_t<u16>  type_u16;
        static const type_t<u32>  type_u32;
        static const type_t<u64>  type_u64;
        static const type_t<s8>   type_s8;
        static const type_t<s16>  type_s16;
        static const type_t<s32>  type_s32;
        static const type_t<s64>  type_s64;
        static const type_t<f32>  type_f32;
        static const type_t<f64>  type_f64;
        static const type_t<bool> type_bool;

        // ------------------------------------------------------------------------------------------------
        // message recipient interface
        struct recipient_t
        {
            virtual void v_receive(system_t* sys, msg_t msg) = 0;
        };

        // ------------------------------------------------------------------------------------------------
        // message system

        struct system_t;
        system_t* create_msg_system();
        void      destroy_msg_system(system_t* sys);

        // --------- message - string registration ---------
        id_t        msg_idof_str(system_t* sys, const char* str);
        const char* msg_strof_id(system_t* sys, id_t id);

        // --------- message - property registration ---------
        template <typename T> property_t msg_register_property(system_t* sys, const char* name, const T& default_value = type_t<T>::default_value);
        template <typename T> property_t msg_register_property(system_t* sys, id_t id, const T& default_value = type_t<T>::default_value);

        template <typename T> void msg_default_property(system_t* sys, property_t property, T const*& outValue);
        id_t                       msg_idof_property(system_t* sys, property_t property);
        s32                        msg_sizeof_property(system_t* sys, property_t property);
        const char*                msg_nameof_property(system_t* sys, property_t property);

        // --------- message - writing ---------
        msg_t msg_begin(system_t* sys, id_t id);
        msg_t msg_begin(system_t* sys, const char* name);

        // You can write many properties as part of a message, they have to be registered though.
        template <typename T> void msg_write(system_t* sys, msg_t msg, property_t pid, T const& value);
        template <typename T> void msg_write(system_t* sys, msg_t msg, const char* pname, T const& value);

        void msg_end(system_t* sys, msg_t msg);

        // --------- message - reading ---------
        void msg_open(system_t* sys, msg_t msg);

        // A message can have many properties, this function will return false when the requested property type
        // is not part of the message. You will get back a pointer to the type, you will have to do something
        // with that data before you call msg_close(msg).
        template <typename T> bool msg_view(system_t* sys, msg_t msg, property_t pid, T const*& value);
        template <typename T> bool msg_view(system_t* sys, msg_t msg, const char* pname, T const*& value);

        void msg_close(system_t* sys, msg_t msg);

        // --------- message - receiver registration ---------
        id_t msg_register_receiver(system_t* sys, const char* url, recipient_t* recv);

        // --------- message - posting ---------
        void msg_post(id_t url_id, msg_t msg);
        void msg_post(const char* url, msg_t msg);

    } // namespace nmsg
} // namespace ncore

#endif // __CMSG_MSG_H__
