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
    class alloc_t;

    namespace nmsg
    {
        typedef u64 id_t;

        struct system_t
        {
            u64 id;
        };
        typedef u64   entity_t;
        typedef u64   component_t;
        typedef u32   msg_t;
        typedef void* property_t;
        typedef void* value_t;

        template <typename T> struct array_t
        {
            void initialize(alloc_t* allocator, u32 capacity)
            {
                m_size     = 0;
                m_capacity = capacity;
                m_data     = (T*)allocator->allocate(sizeof(T) * m_capacity);
            }

            s32 size() const { return m_size; }

            T& checkout()
            {
                ASSERT(m_size < m_capacity);
                return m_data[m_size];
            }
            void commit()
            {
                ASSERT(m_size < m_capacity);
                m_size++;
            }

            void push_back(const T& value)
            {
                checkout() = value;
                commit();
            }

            T&   back() { return m_data[m_size - 1]; }
            void pop_back() { m_size--; }

            void insert(s32 index, const T& value)
            {
                ASSERT(index <= m_size);
                ASSERT(m_size < m_capacity);

                if (index < m_size)
                    nmem::memmove(m_data + index + 1, m_data + index, sizeof(T) * (m_size - index));

                m_data[index] = value;
                m_size++;
            }

            T*  m_data;
            s32 m_size;
            s32 m_capacity;
        };

        struct vector3_t
        {
            float x, y, z;

            static vector3_t zero;
            static vector3_t up;
        };

        struct f32x3
        {
            float x, y, z;

            static f32x3 zero;
            static f32x3 up;
        };

        struct typeinfo_t
        {
            inline typeinfo_t()
                : m_type_name(nullptr)
                , m_default_value(nullptr)
                , m_sizeof(0)
                , m_id(-1)
            {
            }
            inline typeinfo_t(const char* type_name, const void* default_value, u32 sizeof_type)
                : m_type_name(type_name)
                , m_default_value(default_value)
                , m_sizeof(sizeof_type)
                , m_id(-1)
            {
            }

            const char* m_type_name;
            const void* m_default_value;
            u32         m_sizeof;
            s32         m_id;
        };

        template <typename T> struct type_t
        {
            static const T    default_value;
            static typeinfo_t typeinfo;
        };

        // ------------------------------------------------------------------------------------------------
        // types
        static const type_t<u8>    type_u8;
        static const type_t<u16>   type_u16;
        static const type_t<u32>   type_u32;
        static const type_t<u64>   type_u64;
        static const type_t<s8>    type_s8;
        static const type_t<s16>   type_s16;
        static const type_t<s32>   type_s32;
        static const type_t<s64>   type_s64;
        static const type_t<f32>   type_f32;
        static const type_t<f64>   type_f64;
        static const type_t<bool>  type_bool;
        static const type_t<f32x3> type_f32x3;

        // ------------------------------------------------------------------------------------------------
        // id <-> name
        class id_system_t
        {
        public:
            id_t        register_id(const char* name);
            const char* nameof_id(id_t system) const;
        };

        // ------------------------------------------------------------------------------------------------
        // message
        class msg_system_t
        {
        public:
            msg_system_t(id_system_t* id_system, alloc_t* alloc);

            template <typename T> property_t register_property(id_t id, const T& default_value = type_t<T>::default_value);
            template <typename T> property_t register_property(const char* name, const T& default_value = type_t<T>::default_value);

            // message - struct or system type
            template <typename T> void default_propert(property_t property, T const*& outValue);
            id_t                       idof_property(property_t property);
            s32                        sizeof_property(property_t property);
            const char*                nameof_property(property_t property);

            // ------------------------------------------------------------------------------------------------
            // message - begin/end, open/close

            // --------- message - writing ---------
            msg_t begin(id_t id);
            msg_t begin(const char* name);

            // You can write many properties as part of a message, they have to be registered though.
            template <typename T> void write(msg_t msg, T const& value);

            void end(msg_t msg);

            // --------- message - reading ---------
            void open(msg_t msg);

            // A message can have many properties, this function will return false when the requested property type
            // is not part of the message. You will get back a pointer to the type, you will have to do something
            // with that data before you call close(msg).
            template <typename T> bool view(msg_t msg, T const*& value);

            void close(msg_t msg);

            // message - posting
            void post(system_t system, msg_t msg);
            void post(entity_t entity, msg_t msg);

            void post(const char* system, msg_t msg);
            void post(const char* system, const char* entity, msg_t msg);

            id_system_t* m_id_system;
        };

        struct ecs_data_t;

        // ------------------------------------------------------------------------------------------------
        class ecs_system_t
        {
        public:
            virtual void on_register(entity_t entity)   = 0;
            virtual void on_unregister(entity_t entity) = 0;

            virtual void on_msg(msg_t msg)                  = 0;
            virtual void on_msg(entity_t entity, msg_t msg) = 0;
            virtual void on_tick(f32 dt)                    = 0;
        };

        class ecs_t
        {
        public:
            ecs_t(id_system_t* id_system, u32 max_ids, u32 max_systems, u32 max_components, alloc_t* allocator);

            // ------------------------------------------------------------------------------------------------
            // id's

            id_t register_id(const char* name);

            // ------------------------------------------------------------------------------------------------
            // system

            system_t    register_system(ecs_system_t* system, id_t id, u32 max_entities = 1024, u32 max_components = 256);
            system_t    register_system(ecs_system_t* system, const char* name, u32 max_entities = 1024, u32 max_components = 256);
            system_t    find_system(const char* name);
            system_t    find_system(id_t id);
            id_t        idof_system(system_t system);
            const char* nameof_system(system_t system);

            // system - handler (debugging: can encapsulate a system to intercept messages for debugging)

            ecs_system_t* get_system_handler(system_t system);
            void          set_system_handler(system_t system, ecs_system_t* handler);

            // system - entity

            entity_t    create_entity(system_t system, id_t id);
            entity_t    create_entity(system_t system, const char* name = nullptr);
            void        destroy_entity(entity_t entity);
            id_t        idof_entity(entity_t entity);
            const char* nameof_entity(entity_t entity);

            // system - component

            template <typename T> component_t register_component(system_t id, const typeinfo_t* typeinfo = &type_t<T>::typeinfo);
            template <typename T> component_t register_component(system_t id, const char* name);
            template <typename T> component_t register_component(const char* name);
            template <typename T> bool        typeof_component(component_t component, const typeinfo_t* typeinfo = &type_t<T>::typeinfo);
            id_t                              idof_component(component_t component);
            const char*                       nameof_component(component_t component);

            // ------------------------------------------------------------------------------------------------
            // entity - components
            // uses 'T const* default_value = &T::default_value' to recognize type

            template <typename T> void add_component(entity_t entity, component_t component, const T& default_value);
            template <typename T> void add_component(entity_t entity, const char* component, const T& default_value);
            template <typename T> bool has_component(entity_t entity, component_t component);
            template <typename T> bool get_component(entity_t entity, component_t component, T& outValue);
            template <typename T> bool get_component(entity_t entity, component_t component, T*& outValue);
            void                       remove_component(entity_t entity, component_t component);

            msg_system_t* msg;
            ecs_data_t*   data;
        };
    } // namespace nmsg
} // namespace ncore

#endif // __CMSG_MSG_H__
