#include "cbase/c_allocator.h"
#include "cbase/c_context.h"
#include "cbase/c_memory.h"
#include "cbase/c_hbb.h"

#include "cmsg/c_msg.h"

namespace ncore
{
    // The whole idea of entity / component / system is to be able to
    // send messages to a specific entity or a specific component
    // or a specific system
    //
    // The message system leans heavily on the concept of 'frame' allocators
    // to avoid memory fragmentation and to allow for fast allocation and
    // batch based deallocation.
    //

    class SystemShips : public ecs_t::ihandler
    {
    public:
        virtual void on_msg(entity_t entity, component_t component, msg_t msg) {}
    };
    SystemShips* gSystemShips = nullptr;

    class SystemAI : public ecs_t::ihandler
    {
    public:
        virtual void on_msg(entity_t entity, component_t component, msg_t msg) {}
    };
    SystemAI* gSystemAI = nullptr;

    class SystemGfxVfx : public ecs_t::ihandler
    {
    public:
        virtual void on_msg(entity_t entity, component_t component, msg_t msg) {}
    };
    SystemGfxVfx* gSystemGfxVfx = nullptr;

    class SystemAudioSfx : public ecs_t::ihandler
    {
    public:
        virtual void on_msg(entity_t entity, component_t component, msg_t msg) {}
    };
    SystemAudioSfx* gSystemAudioSfx = nullptr;

    class SystemRender : public ecs_t::ihandler
    {
    public:
        virtual void on_msg(entity_t entity, component_t component, msg_t msg) {}
    };
    SystemRender* gSystemRender = nullptr;

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

    static void use_case()
    {
        alloc_t* allocator = context_t::system_alloc();

        ecs_t ecs(allocator);

        system_t    ships_system = ecs.register_system(gSystemShips, "system/ships");
        component_t ship_pos_cp  = ecs.register_component<vector3_t>(ships_system, "position");
        component_t ship_dir_cp  = ecs.register_component<vector3_t>(ships_system, "direction");

        entity_t bug1 = ecs.create_entity(ships_system, "entity/bug1");
        ecs.add_component<vector3_t>(bug1, ship_pos_cp, vector3_t::zero); // by id
        ecs.add_component<vector3_t>(bug1, ship_dir_cp, vector3_t::up);   // by id
        ecs.add_component<f32>(bug1, "speed", 10.0f);                     // by name
        ecs.add_component<f32>(bug1, "radius", 1.0f);                     // by name

        system_t ai_system     = ecs.register_system(gSystemAI, "system/ai");
        system_t vfx_system    = ecs.register_system(gSystemGfxVfx, "system/gfx/vfx");
        system_t sfx_system    = ecs.register_system(gSystemAudioSfx, "system/audio/sfx");
        system_t render_system = ecs.register_system(gSystemRender, "system/gfx/render");

        msg_system& msg = ecs.msg;

        id_t explosion_msg_id = ecs.register_id("msg/explosion");

        property_t pos_prop    = msg.register_property<vector3_t>("position", vector3_t::zero);
        property_t radius_prop = msg.register_property<float>("radius", 10.0f);

        // what about composing a message using named (registered) properties?
        vector3_t explosion_pos(100, 2, 5);

        // position, radius, damage
        msg_t explosion_msg = msg.begin(explosion_msg_id, 3);
        {
            // properties by id
            msg.write_property<vector3_t>(explosion_msg, pos_prop, explosion_pos); // by id
            msg.write_property<f32>(explosion_msg, radius_prop, 10.0f);            // by id
            msg.write_property<f32>(explosion_msg, "damage", 0.9f);                // by name
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

    // the standard types that can be used as ecs components and message properties
    const type_t<u8>        type_u8;
    const type_t<u16>       type_u16;
    const type_t<u32>       type_u32;
    const type_t<u64>       type_u64;
    const type_t<s8>        type_s8;
    const type_t<s16>       type_s16;
    const type_t<s32>       type_s32;
    const type_t<s64>       type_s64;
    const type_t<f32>       type_f32;
    const type_t<bool>      type_bool;
    const type_t<vector3_t> type_vector3;

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
    typeinfo_t type_t<bool>::typeinfo("bool", &type_t<bool>::default_value, sizeof(bool));
    typeinfo_t type_t<vector3_t>::typeinfo("vector3", &type_t<vector3_t>::default_value, sizeof(vector3_t));

    ecs_t::ecs_t(alloc_t* allocator, u32 max_ids, u32 max_systems, u32 max_components)
        : msg(allocator)
    {
    }

    // the name is hashed and stored in an array of ids that is sorted by hash but also contains an index to the name
    // the name is stored in a separate array of strings and not available in the release/final build
    // hash collisions are not allowed

    struct id_data_t
    {
        u32 m_hash;
        u32 m_offset;
    };

    struct named_item_t
    {
        u32 m_index;  // index into the item array
        u32 m_offset; // offset into the name array
    };

    struct component_info_t
    {
        typeinfo_t* m_typeinfo;
    };

    struct system_info_t
    {
        u32              m_max_components;
        ecs_t::ihandler* m_handler;
    };

    template <typename T> struct array_t
    {
        T*  m_data;
        u32 m_size;
        u32 m_capacity;
    };

    struct system_data_t
    {
        ecs_data_t* m_ecs; // pointer to the ecs data

        array_t<u16>   m_component_remap; // set to ecs 'max' components
        array_t<void*> m_component_data;  // set to ecs-system 'max' components

        hbb_data_t   m_entity_used;             // set to ecs-system 'max' entities
        hbb_hdr_t    m_entity_cp_used_hbb_hdr;  // hbb hdr for used components
        array_t<u32> m_entity_cp_used_hbb_data; // entity, hbb data per entity to indicate which component is used
    };

    struct ecs_data_t
    {
        alloc_t* m_allocator;

        // id_t - name
        array_t<id_data_t> m_ids;   // capacity = max-ids
        array_t<char>      m_names; // the names are stored in a single array of chars

        // when registering a component by name, the name is hashed and the hash is used to find the component
        // but then when we insert (sorted) it into the component array, we need to know the index of the component
        hbb_hdr_t                 m_component_free_hbb_hdr;
        hbb_data_t                m_component_free_hbb_data;
        array_t<named_item_t>     m_component_names; // (sorted by hash) capacity = max-components
        array_t<component_info_t> m_component_array; // capacity = max-components

        // when registering a system by name (or getting by name), the name is hashed and the hash is used to find the system
        // but then when we insert (sorted) it into the system array, we need to know the index of the component
        hbb_hdr_t              m_system_free_hbb_hdr;
        hbb_data_t             m_system_free_hbb_data;
        array_t<named_item_t>  m_system_names; // (sorted by hash) capacity = max-components
        array_t<system_info_t> m_system_array; // capacity = max-components
        array_t<system_data_t> m_system_array;
    };

    id_t        ecs_t::register_id(const char* name) {}
    const char* ecs_t::nameof_id(id_t system) {}

} // namespace ncore