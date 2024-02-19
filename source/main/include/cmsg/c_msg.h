#ifndef __CMSG_MSG_H__
#define __CMSG_MSG_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

#include "ccore/c_debug.h"

namespace ncore
{
    class alloc_t;

    namespace nmsg
    {
        typedef u64   id_t;
        typedef u64   system_t;
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
                ASSERT(index <= m_size);
                ASSERT(m_size < m_capacity);
                return m_data[m_size];
            }
            void commit()
            {
                ASSERT(index <= m_size);
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
            vector3_t()
                : x(0)
                , y(0)
                , z(0)
            {
            }
            vector3_t(float x, float y, float z)
                : x(x)
                , y(y)
                , z(z)
            {
            }

            float x, y, z;

            static vector3_t zero;
            static vector3_t up;
        };

        struct typeinfo_t
        {
            typeinfo_t(const char* type_name, const void* default_value, u32 sizeof_type)
                : m_type_name(type_name)
                , m_default_value(default_value)
                , m_sizeof(sizeof_type)
            {
            }
            const char* m_type_name;
            const void* m_default_value;
            u32         m_sizeof;
        };

        template <typename T> struct type_t
        {
            static const T    default_value;
            static typeinfo_t typeinfo;
        };

        // ------------------------------------------------------------------------------------------------
        // types
        static const type_t<u8>        type_u8;
        static const type_t<u16>       type_u16;
        static const type_t<u32>       type_u32;
        static const type_t<u64>       type_u64;
        static const type_t<s8>        type_s8;
        static const type_t<s16>       type_s16;
        static const type_t<s32>       type_s32;
        static const type_t<s64>       type_s64;
        static const type_t<f32>       type_f32;
        static const type_t<bool>      type_bool;
        static const type_t<vector3_t> type_vector3;

        // ------------------------------------------------------------------------------------------------
        // message
        class msg_system
        {
        public:
            msg_system(alloc_t* alloc);

            template <typename T> property_t register_property(id_t id, const T& default_value = type_t<T>::default_value);
            template <typename T> property_t register_property(const char* name, const T& default_value = type_t<T>::default_value);

            // message - property
            template <typename T> void property_default_value(property_t property, T const*& outValue);
            id_t                       idof_property(property_t property);
            s32                        sizeof_property(property_t property);
            const char*                nameof_property(property_t property);

            // ------------------------------------------------------------------------------------------------
            // message - begin/end, open/close

            msg_t begin(id_t id, u32 number_of_properties);
            msg_t begin(const char* name, u32 number_of_properties);
            void  end(msg_t msg);

            // message - write
            template <typename T> value_t write_property(msg_t msg, property_t property, T const& value);
            template <typename T> value_t write_property(msg_t msg, const char* property_name, T const& value);

            // message - read
            s32                        num_properties(msg_t msg);
            template <typename T> bool read_property(msg_t msg, property_t property, T const*& value);
            value_t                    get_property(msg_t msg, property_t property);
            template <typename T> bool is_property_typeof(msg_t msg, property_t property, const typeinfo_t* typeinfo = &type_t<T>::typeinfo);

            // message - post
            // this could increase a ref-count, so that we can know when to release the message
            void post(system_t system, msg_t msg);
            void post(entity_t entity, msg_t msg);
            void post(entity_t entity, component_t component, msg_t msg);

            void post(const char* system, msg_t msg);
            void post(const char* system, const char* entity, msg_t msg);
            void post(const char* system, const char* entity, const char* component, msg_t msg);
        };

        struct ecs_data_t;

        // ------------------------------------------------------------------------------------------------
        class ecs_system_t
        {
        public:
            virtual void on_register(entity_t entity)   = 0;
            virtual void on_unregister(entity_t entity) = 0;

            virtual void on_msg(entity_t entity, component_t component, msg_t msg) = 0;
            virtual void on_tick(f32 dt)                                           = 0;
        };

        class ecs_t
        {
        public:
            ecs_t(alloc_t* allocator, u32 max_ids = 65536, u32 max_systems = 256, u32 max_components = 4096);

            // ------------------------------------------------------------------------------------------------
            // id

            id_t        register_id(const char* name);
            const char* nameof_id(id_t system);

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

            template <typename T> component_t register_component(system_t system, id_t id, const typeinfo_t* typeinfo = &type_t<T>::typeinfo);
            template <typename T> component_t register_component(system_t system, const char* name, const typeinfo_t* typeinfo = &type_t<T>::typeinfo);
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

            msg_system  msg;
            ecs_data_t* data;
        };
    } // namespace nmsg
} // namespace ncore

#endif // __CMSG_MSG_H__