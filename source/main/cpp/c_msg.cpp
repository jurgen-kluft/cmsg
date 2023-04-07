#include "cbase/c_allocator.h"
#include "cbase/c_context.h"
#include "cbase/c_memory.h"

#include "cmsg/c_msg.h"

namespace ncore
{
    // The whole idea of entity / component / system is to be able to
    // send messages to a specific entity or a specific component
    // or a specific system
    //
    // The message system leans heavily on the concept of 'frame' allocators
    // to avoid memory fragmentation and to allow for fast allocation and no deallocation.
    //

    class SystemShips : public ecs_system::ihandler
    {
    public:
            virtual void on_msg(entity_t entity, component_t component, msg_t msg)
            {

            }
    };
    SystemShips* gSystemShips = nullptr;

    class SystemAI : public ecs_system::ihandler
    {
    public:
            virtual void on_msg(entity_t entity, component_t component, msg_t msg)
            {
                
            }
    };
    SystemAI* gSystemAI = nullptr;

    class SystemGfxVfx : public ecs_system::ihandler
    {
    public:
            virtual void on_msg(entity_t entity, component_t component, msg_t msg)
            {
                
            }
    };
    SystemGfxVfx* gSystemGfxVfx = nullptr;

    class SystemAudioSfx : public ecs_system::ihandler
    {
    public:
            virtual void on_msg(entity_t entity, component_t component, msg_t msg)
            {
                
            }
    };
    SystemAudioSfx* gSystemAudioSfx = nullptr;

    class SystemRender : public ecs_system::ihandler
    {
    public:
            virtual void on_msg(entity_t entity, component_t component, msg_t msg)
            {
                
            }
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

        ecs_system ecs(allocator);

        system_t    ships_system = ecs.register_system(gSystemShips, "system/ships");
        component_t ship_pos_cp  = ecs.register_component<vector3_t>(ships_system, "position");
        component_t ship_dir_cp  = ecs.register_component<vector3_t>(ships_system, "direction");

        entity_t bug1 = ecs.create_entity(ships_system, "entity/bug1");
        ecs.add_component<vector3_t>(bug1, ship_pos_cp, vector3_t::zero); // by id
        ecs.add_component<vector3_t>(bug1, ship_dir_cp, vector3_t::up);   // by id
        ecs.add_component<f32>(bug1, "speed", 10.0f);                   // by name
        ecs.add_component<f32>(bug1, "radius", 1.0f);                   // by name

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
            msg.write_property<f32>(explosion_msg, radius_prop, 10.0f);          // by id
            msg.write_property<f32>(explosion_msg, "damage", 0.9f);              // by name
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

} // namespace ncore