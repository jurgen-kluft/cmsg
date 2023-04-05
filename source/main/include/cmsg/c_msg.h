#ifndef __CMSG_MSG_H__
#define __CMSG_MSG_H__
#include "cbase/c_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

#include "cbase/c_debug.h"

namespace ncore
{
    class cmsg
    {
    public:
        typedef u32   id_t;        // 32
        typedef u32   system_t;    // 9 bits for system
        typedef u32   entity_t;    // 9 bits for system, 18 bits for entity, 5 bits for salt
        typedef u32   component_t; // 9 bits for system, 18 bits for component, 5 bits for salt
        typedef u32   msg_t;
        typedef void* property_t;
        typedef void* value_t;

        const entity_t kEntityInvalid = 0;

        const entity_t    kEntityAll        = 0xffffffffffffffff;
        const component_t kComponentAll     = 0xffffffff;
        const char* const kEntityNameAll    = "*";
        const char* const kComponentNameAll = "*";

        template <typename T> struct default_component_t
        {
            static T default_value;
        };
        const default_component_t<u8>   default_cp_u8;
        const default_component_t<u16>  default_cp_u16;
        const default_component_t<u32>  default_cp_u32;
        const default_component_t<u64>  default_cp_u64;
        const default_component_t<s8>   default_cp_s8;
        const default_component_t<s16>  default_cp_s16;
        const default_component_t<s32>  default_cp_s32;
        const default_component_t<s64>  default_cp_s64;
        const default_component_t<f32>  default_cp_f32;
        const default_component_t<bool> default_cp_bool;

        template <typename T> struct default_property_t
        {
            static T default_value;
        };
        const default_property_t<u8>   default_prop_u8;
        const default_property_t<u16>  default_prop_u16;
        const default_property_t<u32>  default_prop_u32;
        const default_property_t<u64>  default_prop_u64;
        const default_property_t<s8>   default_prop_s8;
        const default_property_t<s16>  default_prop_s16;
        const default_property_t<s32>  default_prop_s32;
        const default_property_t<s64>  default_prop_s64;
        const default_property_t<f32>  default_prop_f32;
        const default_property_t<bool> default_prop_bool;

        // ------------------------------------------------------------------------------------------------
        class ihandler
        {
        public:
            virtual void on_msg(entity_t entity, component_t component, msg_t msg) = 0;
        };

        // ------------------------------------------------------------------------------------------------
        // id

        id_t        register_id(const char* name);
        const char* nameof_id(id_t system);

        // ------------------------------------------------------------------------------------------------
        // system

        system_t    register_system(IMsgHandler* system, id_t id, u32 max_entities = 1024, u32 max_components = 256);
        system_t    register_system(IMsgHandler* system, const char* name, u32 max_entities = 1024, u32 max_components = 256);
        id_t        idof_system(system_t system);
        const char* nameof_system(system_t system);

        // system - handler (debugging: can encapsulate a system to intercept messages for debugging)

        IMsgHandler* get_system_handler(system_t system);
        void         set_system_handler(system_t system, IMsgHandler* handler);

        // system - entity

        entity_t    create_entity(system_t system, id_t id = kEntity);
        entity_t    create_entity(system_t system, const char* name = kEntityNameAll);
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

        template <typename T> void add_component(entity_t entity, component_t component, const T* default_value = &default_component_t<T>::default_value);
        template <typename T> void add_component(entity_t entity, const char* component, const T* default_value = &default_component_t<T>::default_value);
        template <typename T> bool is_typeof(entity_t entity, component_t component, const T* default_value = &default_component_t<T>::default_value);
        template <typename T> bool has_component(entity_t entity, component_t component);
        template <typename T> bool get_component(entity_t entity, component_t component, T& outValue);
        template <typename T> bool get_component(entity_t entity, component_t component, T*& outValue);
        void                       remove_component(entity_t entity, component_t component);

        // ------------------------------------------------------------------------------------------------
        // message
        // uses 'T const* default_value = &T::default_value' to recognize property type

        template <typename T> property_t register_property(id_t id, const T* default_value = &default_property_t<T>::default_value);
        template <typename T> property_t register_property(const char* name, const T* default_value = &default_property_t<T>::default_value);

        // message - property
        template <typename T> void property_default_value(property_t property, T const*& outValue);
        id_t                       idof_property(property_t property);
        s32                        sizeof_property(property_t property);
        const char*                nameof_property(property_t property);

        // ------------------------------------------------------------------------------------------------
        // message - begin/end, open/close

        msg_t begin_msg(id_t id, u32 number_of_properties);
        msg_t begin_msg(const char* name, u32 number_of_properties);
        void  end_msg(msg_t msg);

        // message - write/read

        template <typename T> value_t write_property(msg_t msg, property_t property, T const& value);
        s32                           num_properties(msg_t msg);
        value_t                       get_property(msg_t msg, property_t property);
        bool                          is_typeof_property(msg_t msg, property_t property, const T* default_value = &default_property_t<T>::default_value);
        template <typename T> bool    read_property(msg_t msg, property_t property, T const*& value);

        // message - post

        post(system_t system, msg_t msg);
        post(entity_t entity, msg_t msg);
        post(entity_t entity, component_t component, msg_t msg);

        post(const char* system, msg_t msg);
        post(const char* system, const char* entity, msg_t msg);
        post(const char* system, const char* entity, const char* component, msg_t msg);

        /*

        Note: Using strings for system, entity and component names is not recommended for performance reasons.

        system_t ships_system = register_system(gSystemShips, "system/ships");
        component_t ship_pos_cp = register_component<vector3_t>(ships_system, "position", sizeof(vector3_t));
        component_t ship_dir_cp = register_component<vector3_t>(ships_system, "direction", sizeof(vector3_t));
        component_t ship_vel_cp = register_component<float>(ships_system, "speed", sizeof(float));
        component_t ship_radius_cp = register_component<float>(ships_system, "radius", sizeof(float));

        entity_t bug1 = create_entity(ships_system, "entity/bug1");
        add_component<vector3_t>(bug1, ship_pos_cp);
        add_component<vector3_t>(bug1, ship_dir_cp, vector3_t::up);
        add_component<float>(bug1, ship_vel_cp, 10.0f);
        add_component<float>(bug1, ship_radius_cp, 1.0f);

        system_t ai_system = register_system(gSystemAI, "system/ai");
        system_t vfx_system = register_system(gSystemGfxVfx, "system/gfx/vfx");
        system_t sfx_system = register_system(gSystemAudioSfx, "system/audio/sfx");
        system_t render_system = register_system(gSystemRender, "system/gfx/render");

        property_t pos_prop = registery_property<vector3_t>("position", vector3_t::zero);
        property_t radius_prop = registery_property<float>("radius", 10.0f);
        property_t damage_prop = registery_property<float>("damage", 1.0f);

        // what about composing a message using named (registered) properties?
        vector3_t explosion_pos(100, 2, 5);

        // position, radius, damage
        explosion_msg = begin_msg("msg/explosion", 3);
        {
            // properties by id
            write_property<vector3_t>(explosion_msg, pos_prop, explosion_pos);
            write_property<float>(explosion_msg, radius_prop, 10.0f);
            write_property<float>(explosion_msg, damage_prop, 0.9f);

            // or 
            
            // properties by name
            write_property<vector3_t>(explosion_msg, "position", explosion_pos);
            write_property<float>(explosion_msg, "radius", 10.0f);
            write_property<float>(explosion_msg, "damage", 0.9f);
        }
        end_msg(explosion_msg);

        // systems by name
        cmsg::post("system/ships", explosion_msg);
        cmsg::post("system/ships", "entity/bug1", explosion_msg);

        // systems by id
        cmsg::post(ships_system, explosion_msg);
        cmsg::post(ai_system, explosion_msg);
        cmsg::post(vfx_system, explosion_msg);
        cmsg::post(sfx_system, explosion_msg);
        */

    } // namespace cmsg
} // namespace ncore

#endif // __CMSG_MSG_H__