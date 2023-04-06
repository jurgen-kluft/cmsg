#ifndef __CMSG_MSG_H__
#define __CMSG_MSG_H__
#include "cbase/c_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

#include "cbase/c_debug.h"

namespace ncore
{
    typedef u32   id_t;        // 32
    typedef u32   system_t;    // 9 bits for system
    typedef u32   entity_t;    // 9 bits for system, 18 bits for entity, 5 bits for salt
    typedef u32   component_t; // 9 bits for system, 18 bits for component, 5 bits for salt
    typedef u32   msg_t;
    typedef void* property_t;
    typedef void* value_t;


    // ------------------------------------------------------------------------------------------------
    // message
    class msg_system
    {
    public:
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

        // message - write/read

        s32                           num_properties(msg_t msg);
        template <typename T> value_t write_property(msg_t msg, property_t property, T const& value);
        template <typename T> bool    read_property(msg_t msg, property_t property, T const*& value);
        value_t                       get_property(msg_t msg, property_t property);
        bool                          is_property_typeof(msg_t msg, property_t property, const T* default_value = &type_t<T>::default_value);

        // message - post

        void post(system_t system, msg_t msg);
        void post(entity_t entity, msg_t msg);
        void post(entity_t entity, component_t component, msg_t msg);

        void post(const char* system, msg_t msg);
        void post(const char* system, const char* entity, msg_t msg);
        void post(const char* system, const char* entity, const char* component, msg_t msg);
    };


    class ecs_system
    {
    public:
        const entity_t    kEntityInvalid    = 0;
        const entity_t    kEntityAll        = 0xffffffffffffffff;
        const component_t kComponentAll     = 0xffffffff;
        const char* const kEntityNameAll    = "*";
        const char* const kComponentNameAll = "*";

        template <typename T> struct type_t
        {
            static T default_value;
        };

        const type_t<u8>   type_u8;
        const type_t<u16>  type_u16;
        const type_t<u32>  type_u32;
        const type_t<u64>  type_u64;
        const type_t<s8>   type_s8;
        const type_t<s16>  type_s16;
        const type_t<s32>  type_s32;
        const type_t<s64>  type_s64;
        const type_t<f32>  type_f32;
        const type_t<bool> type_bool;

        // ------------------------------------------------------------------------------------------------
        class ihandler
        {
        public:
            virtual void on_msg(entity_t entity, component_t component, msg_t msg) = 0;
        };

        ecs_system(alloc_t* allocator);

        // ------------------------------------------------------------------------------------------------
        // id

        id_t        register_id(const char* name);
        const char* nameof_id(id_t system);

        // ------------------------------------------------------------------------------------------------
        // system

        system_t    register_system(ihandler* system, id_t id, u32 max_entities = 1024, u32 max_components = 256);
        system_t    register_system(ihandler* system, const char* name, u32 max_entities = 1024, u32 max_components = 256);
        system_t    find_system(const char* name);
        system_t    find_system(id_t id);
        id_t        idof_system(system_t system);
        const char* nameof_system(system_t system);

        // system - handler (debugging: can encapsulate a system to intercept messages for debugging)

        ihandler* get_system_handler(system_t system);
        void      set_system_handler(system_t system, ihandler* handler);

        // system - entity

        entity_t    create_entity(system_t system, id_t id);
        entity_t    create_entity(system_t system, const char* name);
        void        destroy_entity(entity_t entity);
        id_t        idof_entity(entity_t entity);
        const char* nameof_entity(entity_t entity);

        // system - component

        template <typename T> component_t register_component(system_t system, id_t id, const T* default_value = &default_component_t<T>::default_value);
        template <typename T> component_t register_component(system_t system, const char* name, const T* default_value = &default_component_t<T>::default_value);
        id_t                              idof_component(component_t component);
        const char*                       nameof_component(component_t component);

        // ------------------------------------------------------------------------------------------------
        // entity - components
        // uses 'T const* default_value = &T::default_value' to recognize type

        template <typename T> void add_component(entity_t entity, component_t component, const T& default_value);
        template <typename T> void add_component(entity_t entity, const char* component, const T& default_value);
        template <typename T> bool is_typeof(entity_t entity, component_t component, const T* default_value = &default_component_t<T>::default_value);
        template <typename T> bool has_component(entity_t entity, component_t component);
        template <typename T> bool get_component(entity_t entity, component_t component, T& outValue);
        template <typename T> bool get_component(entity_t entity, component_t component, T*& outValue);
        void                       remove_component(entity_t entity, component_t component);

        msg_system msg;
    };

    /*

    Note: Using strings for system, entity and component names is not recommended for performance reasons.
    ecs_system ecs(allocator);

    system_t ships_system      = ecs.register_system(gSystemShips, "system/ships");
    component_t ship_pos_cp    = ecs.register_component<vector3_t>(ships_system, "position", sizeof(vector3_t));
    component_t ship_dir_cp    = ecs.register_component<vector3_t>(ships_system, "direction", sizeof(vector3_t));
    component_t ship_vel_cp    = ecs.register_component<float>(ships_system, "speed", sizeof(float));
    component_t ship_radius_cp = ecs.register_component<float>(ships_system, "radius", sizeof(float));

    entity_t bug1 = ecs.create_entity(ships_system, "entity/bug1");
    ecs.add_component<vector3_t>(bug1, ship_pos_cp);
    ecs.add_component<vector3_t>(bug1, ship_dir_cp, vector3_t::up);
    ecs.add_component<float>(bug1, ship_vel_cp, 10.0f);
    ecs.add_component<float>(bug1, ship_radius_cp, 1.0f);

    system_t ai_system     = ecs.register_system(gSystemAI, "system/ai");
    system_t vfx_system    = ecs.register_system(gSystemGfxVfx, "system/gfx/vfx");
    system_t sfx_system    = ecs.register_system(gSystemAudioSfx, "system/audio/sfx");
    system_t render_system = ecs.register_system(gSystemRender, "system/gfx/render");

    msg_system msg = ecs.msg();

    id_t explosion_msg_id = msg.register_id("msg/explosion");

    property_t pos_prop    = msg.registery_property<vector3_t>("position", vector3_t::zero);
    property_t radius_prop = msg.registery_property<float>("radius", 10.0f);
    property_t damage_prop = msg.registery_property<float>("damage", 1.0f);

    // what about composing a message using named (registered) properties?
    vector3_t explosion_pos(100, 2, 5);

    // position, radius, damage
    explosion_msg = msg.begin(explosion_msg_id, 3);
    {
        // properties by id
        msg.write_property<vector3_t>(explosion_msg, pos_prop, explosion_pos);
        msg.write_property<float>(explosion_msg, radius_prop, 10.0f);
        msg.write_property<float>(explosion_msg, damage_prop, 0.9f);

        // or

        // properties by name
        msg.write_property<vector3_t>(explosion_msg, "position", explosion_pos);
        msg.write_property<float>(explosion_msg, "radius", 10.0f);
        msg.write_property<float>(explosion_msg, "damage", 0.9f);
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
    */

} // namespace ncore

#endif // __CMSG_MSG_H__