#include "ccore/c_allocator.h"
#include "ccore/c_debug.h"
#include "cbase/c_context.h"
#include "cbase/c_debug.h"
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
        // batch based deallocation.
        //

        class SystemShips : public ecs_system_t
        {
        public:
            virtual void on_msg(entity_t entity, component_t component, msg_t msg) {}
            virtual void on_tick(f32 dt) {}

            virtual void on_register(entity_t entity) {}
            virtual void on_unregister(entity_t entity) {}
        };
        SystemShips* gSystemShips = nullptr;

        class SystemAI : public ecs_system_t
        {
        public:
            virtual void on_msg(entity_t entity, component_t component, msg_t msg) {}
            virtual void on_tick(f32 dt) {}

            virtual void on_register(entity_t entity) {}
            virtual void on_unregister(entity_t entity) {}
        };
        SystemAI* gSystemAI = nullptr;

        class SystemGfxVfx : public ecs_system_t
        {
        public:
            virtual void on_msg(entity_t entity, component_t component, msg_t msg) {}
            virtual void on_tick(f32 dt) {}

            virtual void on_register(entity_t entity) {}
            virtual void on_unregister(entity_t entity) {}
        };
        SystemGfxVfx* gSystemGfxVfx = nullptr;

        class SystemAudioSfx : public ecs_system_t
        {
        public:
            virtual void on_msg(entity_t entity, component_t component, msg_t msg) {}
            virtual void on_tick(f32 dt) {}

            virtual void on_register(entity_t entity) {}
            virtual void on_unregister(entity_t entity) {}
        };
        SystemAudioSfx* gSystemAudioSfx = nullptr;

        class SystemRender : public ecs_system_t
        {
        public:
            virtual void on_msg(entity_t entity, component_t component, msg_t msg) {}
            virtual void on_tick(f32 dt) {}

            virtual void on_register(entity_t entity) {}
            virtual void on_unregister(entity_t entity) {}
        };
        SystemRender* gSystemRender = nullptr;

        static void use_case()
        {
            alloc_t* allocator = context_t::system_alloc();

            id_system_t id_system;
            ecs_t ecs(&id_system, 65536, 1024, 1024, allocator);
            id_system_t ids;

            system_t    ships_system = ecs.register_system(gSystemShips, "system/ships");
            component_t ship_pos_cp  = ecs.register_component<f32x3>("position");
            component_t ship_dir_cp  = ecs.register_component<f32x3>("direction");

            entity_t bug1 = ecs.create_entity(ships_system, "entity/bug1");
            ecs.add_component<f32x3>(bug1, ship_pos_cp, f32x3::zero); // by id
            ecs.add_component<f32x3>(bug1, ship_dir_cp, f32x3::up);   // by id
            ecs.add_component<f32>(bug1, "speed", 10.0f);             // by name
            ecs.add_component<f32>(bug1, "radius", 1.0f);             // by name

            system_t ai_system     = ecs.register_system(gSystemAI, "system/ai");
            system_t vfx_system    = ecs.register_system(gSystemGfxVfx, "system/gfx/vfx");
            system_t sfx_system    = ecs.register_system(gSystemAudioSfx, "system/audio/sfx");
            system_t render_system = ecs.register_system(gSystemRender, "system/gfx/render");

            msg_system_t& msg = ecs.msg;

            id_t explosion_msg_id = ids.register_id("msg/explosion");

            property_t pos_prop    = msg.register_property<f32x3>("position", f32x3::zero);
            property_t radius_prop = msg.register_property<float>("radius", 10.0f);

            // what about composing a message using named (registered) properties?
            f32x3 explosion_pos {100, 2, 5};

            // position, radius, damage
            msg_t explosion_msg = msg.begin(explosion_msg_id);
            {
                // properties by id
                msg.write_property<f32x3>(explosion_msg, pos_prop, explosion_pos); // by id
                msg.write_property<f32>(explosion_msg, radius_prop, 10.0f);        // by id
                msg.write_property<f32>(explosion_msg, "damage", 0.9f);            // by name
            }
            msg.end(explosion_msg);

            // systems by name
            msg.post("system/ships", explosion_msg);
            msg.post("system/ships", "entity/bug1", explosion_msg);

            // systems by id
            msg.post(ships_system, explosion_msg);
            msg.post(ai_system, explosion_msg);
            msg.post(vfx_system, explosion_msg);
            msg.post(sfx_system, explosion_msg);
        }

        // the defaults of the above types are:
        const u8    type_t<u8>::default_value    = 0;
        const u16   type_t<u16>::default_value   = 0;
        const u32   type_t<u32>::default_value   = 0;
        const u64   type_t<u64>::default_value   = 0;
        const s8    type_t<s8>::default_value    = 0;
        const s16   type_t<s16>::default_value   = 0;
        const s32   type_t<s32>::default_value   = 0;
        const s64   type_t<s64>::default_value   = 0;
        const f32   type_t<f32>::default_value   = 0.0f;
        const f64   type_t<f64>::default_value   = 0.0f;
        const bool  type_t<bool>::default_value  = false;
        const f32x3 type_t<f32x3>::default_value = f32x3::zero;

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
        typeinfo_t type_t<f32x3>::typeinfo("f32x3", &type_t<f32x3>::default_value, sizeof(f32x3));

        template <typename T> struct array_t
        {
            void initialize(alloc_t* allocator, u32 capacity)
            {
                m_size     = 0;
                m_capacity = capacity;
                m_data     = (T*)allocator->allocate(sizeof(T) * m_capacity);
            }

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

        static s32 binary_search(const u32* array, u32 size, u32 value)
        {
            s32 low  = 0;
            s32 high = size - 1;

            while (low <= high)
            {
                s32 mid       = (low + high) / 2;
                u32 mid_value = array[mid];

                if (mid_value < value)
                    low = mid + 1;
                else if (mid_value > value)
                    high = mid - 1;
                else
                    return mid;
            }

            return low; // return the index where the value should be inserted
        }

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
                m_name_offset_array.initialize(allocator, max_names);
                m_name_hash_array.initialize(allocator, max_names);

                // initialize the name string array assuming that the average name length is 32
                m_name_str_array.initialize(allocator, max_names * 32);
            }

            static u32 generate_hash(const char* name)
            {
                u64 const seed  = 0;
                u64 const prime = 0x9E3779B185EBCA87ULL;
                u64       hash  = seed + prime;
                while (*name)
                {
                    hash ^= *name++;
                    hash *= prime;
                }
                return (u32)hash;
            }

            u32 register_name(const char* name)
            {
                u32 const hash = generate_hash(name);

                // find the name in the hash array which is sorted by hash
                const s32 index = binary_search(m_name_hash_array.m_data, m_name_hash_array.m_size, hash);

                // if the name is already registered return the id
                if (index < m_name_hash_array.m_size && m_name_hash_array.m_data[index] == hash)
                {
                    ASSERT(ascii::compare(name, m_name_str_array.m_data + m_name_offset_array.m_data[index]) == 0);
                    return m_name_offset_array.m_data[index];
                }

                // we assume that we do not have hash collisions, so a hash is unique

                // if the name is not registered, insert it in the hash array
                // make space for the new name in the hash and offset array
                m_name_hash_array.insert(index, hash);
                m_name_offset_array.insert(index, m_name_str_array.m_size);

                // append the name in the string array
                while (*name && m_name_str_array.m_size < m_name_str_array.m_capacity)
                    m_name_str_array.m_data[m_name_str_array.m_size++] = *name++;
            }

            array_t<u32>  m_name_hash_array;   // capacity = max-ids
            array_t<u32>  m_name_offset_array; // capacity = max-ids
            array_t<char> m_name_str_array;    // the names are stored in a single array of chars
        };

        ecs_t::ecs_t(id_system_t* id_system, u32 max_ids, u32 max_systems, u32 max_components, alloc_t* allocator)
            : msg(id_system, allocator)
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
            ecs_data_t*        m_ecs; // pointer to the ecs data

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

        id_t        id_system_t::register_id(const char* name) { return id_t{0}; }
        void        id_system_t::unregister_id(id_t id) {}
        const char* id_system_t::nameof_id(id_t system) const { return ""; }
    } // namespace nmsg
} // namespace ncore