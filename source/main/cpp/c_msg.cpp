#include "ccore/c_allocator.h"
#include "ccore/c_debug.h"
#include "cbase/c_context.h"
#include "cbase/c_memory.h"
#include "cbase/c_hbb.h"
#include "cbase/c_runes.h"

#include "cmsg/c_msg.h"

namespace ncore
{
    namespace nmsg
    {
        // The whole idea of entity / component / system is to be able to
        // send messages to a specific entity or a specific component
        // or a specific system
        //
        // The message system leans heavily on the concept of 'frame' allocators
        // to avoid memory fragmentation and to allow for fast allocation and
        // batch or no-op based deallocation.
        //

        // the defaults of the above types are:
        const u8        type_t<u8>::default_value        = 0;
        const u16       type_t<u16>::default_value       = 0;
        const u32       type_t<u32>::default_value       = 0;
        const u64       type_t<u64>::default_value       = 0;
        const s8        type_t<s8>::default_value        = 0;
        const s16       type_t<s16>::default_value       = 0;
        const s32       type_t<s32>::default_value       = 0;
        const s64       type_t<s64>::default_value       = 0;
        const f32       type_t<f32>::default_value       = 0.0f;
        const f64       type_t<f64>::default_value       = 0.0;
        const bool      type_t<bool>::default_value      = false;
        const vector3_t type_t<vector3_t>::default_value = vector3_t::zero;

        typeinfo_t type_t<u8>::typeinfo("u8", &type_t<u8>::default_value, sizeof(u8));
        typeinfo_t type_t<u16>::typeinfo("u16", &type_t<u16>::default_value, sizeof(u16));
        typeinfo_t type_t<u32>::typeinfo("u32", &type_t<u32>::default_value, sizeof(u32));
        typeinfo_t type_t<u64>::typeinfo("u64", &type_t<u64>::default_value, sizeof(u64));
        typeinfo_t type_t<s8>::typeinfo("s8", &type_t<s8>::default_value, sizeof(s8));
        typeinfo_t type_t<s16>::typeinfo("s16", &type_t<s16>::default_value, sizeof(s16));
        typeinfo_t type_t<s32>::typeinfo("s32", &type_t<s32>::default_value, sizeof(s32));
        typeinfo_t type_t<s64>::typeinfo("s64", &type_t<s64>::default_value, sizeof(s64));
        typeinfo_t type_t<f32>::typeinfo("f32", &type_t<f32>::default_value, sizeof(f32));
        typeinfo_t type_t<f64>::typeinfo("f64", &type_t<f64>::default_value, sizeof(f64));
        typeinfo_t type_t<bool>::typeinfo("bool", &type_t<bool>::default_value, sizeof(bool));
        typeinfo_t type_t<vector3_t>::typeinfo("vector3", &type_t<vector3_t>::default_value, sizeof(vector3_t));

        //          name registration (ASCII only)
        // NOTE:
        // do we really have to store the strings ourselves or could we just
        // use the pointers ?
        // if the user only uses direct strings, e.g. ecs.register_id("speed");
        // then we could just use the string pointers (and no need for hashing)
        struct names_db_t
        {
            void init(alloc_t* allocator, u32 max_names = 32768)
            {
                m_name_pointer_array.initialize(allocator, max_names);
                m_name_index_array.initialize(allocator, max_names);
            }

            u32 register_name(const char* name)
            {
                // find the name in the hash array which is sorted by hash
                const s32 index = binary_search(name);
                if (index >= 0)
                    return index;
                if (index < m_name_pointer_array.m_size && m_name_pointer_array.m_data[index] == name)
                    return index;

                m_name_index_array.insert(index, m_name_pointer_array.m_size);
                m_name_pointer_array.add(name);
            }

            s32 binary_search(const char* value) const
            {
                s32 low  = 0;
                s32 high = m_name_index_array.m_size - 1;

                while (low <= high)
                {
                    const s32   mid       = (low + high) / 2;
                    const char* mid_value = m_name_pointer_array.m_data[m_name_index_array.m_data[mid]];

                    if (mid_value < value)
                        low = mid + 1;
                    else if (mid_value > value)
                        high = mid - 1;
                    else
                        return mid;
                }

                return low; // return the index where the value should be inserted
            }

            array_t<const char*> m_name_pointer_array; // capacity = max-ids (non-sorted)
            array_t<s32>         m_name_index_array;   // capacity = max-ids (sorted by 'const char*' pointer)
        };

        ecs_t::ecs_t(alloc_t* allocator, u32 max_ids, u32 max_systems, u32 max_components)
            : msg(allocator)
            , data(nullptr)
        {
        }

        // the name is hashed and stored in an array of ids that is sorted by hash but also contains an index to the name
        // the name is stored in a separate array of strings and not available in the release/final build
        // hash collisions are not allowed

        struct name_data_t
        {
            u32 m_hash;   // hash of the name (name hash array)
            u32 m_offset; // offset of the name (name str array)
            u32 m_index;  // index into the object array (component, system, ...)
        };

        struct component_data_t
        {
            typeinfo_t* m_typeinfo;
        };

        struct system_data_t
        {
            ecs_system_t* m_handler;
            ecs_data_t*   m_ecs; // pointer to the ecs data

            u32            m_max_components;
            array_t<u16>   m_component_remap; // set to ecs 'max' components
            array_t<void*> m_component_data;  // set to ecs-system 'max' components

            hbb_data_t   m_entity_used;             // set to ecs-system 'max' entities
            hbb_hdr_t    m_entity_cp_used_hbb_hdr;  // hbb hdr for used components
            array_t<u32> m_entity_cp_used_hbb_data; // entity, hbb data per entity to indicate which component is used
        };

        struct ecs_data_t
        {
            alloc_t* m_allocator;

            names_db_t m_names;

            // when registering a component by name, the name is hashed and the hash is used to find the component
            // but then when we insert (sorted) it into the component array, we need to know the index of the component
            hbb_hdr_t                 m_component_free_hbb_hdr;
            hbb_data_t                m_component_free_hbb_data;
            array_t<name_data_t>      m_component_names; // (sorted by hash) capacity = max-components
            array_t<component_data_t> m_component_array; // capacity = max-components

            // when registering a system by name (or getting by name), the name is hashed and the hash is used to find the system
            // but then when we insert (sorted) it into the system array, we need to know the index of the component
            hbb_hdr_t              m_system_free_hbb_hdr;
            hbb_data_t             m_system_free_hbb_data;
            array_t<name_data_t>   m_system_names; // (sorted by hash) capacity = max-components
            array_t<system_data_t> m_system_array; // capacity = max-components
        };

        id_t        ecs_t::register_id(const char* name) { return id_t{0}; }
        const char* ecs_t::nameof_id(id_t system) { return ""; }
    } // namespace nmsg
} // namespace ncore