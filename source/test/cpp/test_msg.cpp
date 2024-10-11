#include "ccore/c_target.h"
#include "cbase/c_context.h"
#include "cbase/c_hbb.h"
#include "cmsg/c_msg.h"

#include "cunittest/cunittest.h"

namespace ncore
{
    using namespace nmsg;

    class SystemShips : public ecs_system_t
    {
        // NOTE: One bit per entity, this array holds the globally maximum number of entities
        binmap_t ent_hbb;

        struct queued_msg_t
        {
            entity_t    ent;
            component_t cmp;
            msg_t       msg;
        };

        array_t<queued_msg_t> msg_queue;

    public:
        virtual void on_register(entity_t entity)
        {
            // set bit for entity
        }

        virtual void on_unregister(entity_t entity)
        {
            // reset bit for entity
        }

        virtual void on_msg(entity_t entity, component_t component, msg_t msg) { msg_queue.checkout() = {entity, component, msg}; msg_queue.commit(); }

        virtual void on_tick(float dt)
        {
            // go over all the queued messages and process them
            while (msg_queue.size() > 0)
            {
                const queued_msg_t& queued_msg = msg_queue.back();

                // handle the message
                // - move + collision (all entities)
                // - aim (all entities)

                // reset the message
                msg_queue.pop_back();
            }
            // message queue is now empty

            // process these actions:
            // - move + collision (all entities)
            // - aim (all entities)

        }
    };
    SystemShips* gSystemShips = nullptr;

    class SystemAI : public ecs_system_t
    {
    public:
        virtual void on_msg(entity_t entity, component_t component, msg_t msg)
        {
            // handle the message
        }

        virtual void on_tick(f32 dt)
        {
            // compute/determine the direction and speed of each entity

        }
    };
    SystemAI* gSystemAI = nullptr;

    class SystemGfxVfx : public ecs_system_t
    {
    public:
        virtual void on_msg(entity_t entity, component_t component, msg_t msg)
        {
            // handle the message
        }

        virtual void on_tick(f32 dt)
        {
            // compute/determine the direction and speed of each entity

        }
    };
    SystemGfxVfx* gSystemGfxVfx = nullptr;

    class SystemAudioSfx : public ecs_system_t
    {
    public:
        virtual void on_msg(entity_t entity, component_t component, msg_t msg)
        {
            // handle the message
        }

        virtual void on_tick(f32 dt)
        {
            // compute/determine the direction and speed of each entity

        }
    };
    SystemAudioSfx* gSystemAudioSfx = nullptr;

    class SystemRender : public ecs_system_t
    {
    public:
        virtual void on_msg(entity_t entity, component_t component, msg_t msg)
        {
            // handle the message
        }

        virtual void on_tick(f32 dt)
        {
            // compute/determine the direction and speed of each entity

        }
    };
    SystemRender* gSystemRender = nullptr;

    static void use_case()
    {
        alloc_t* allocator = context_t::system_alloc();
        id_system_t id_system;

        ecs_t ecs(&id_system, 1024, 256, 256, allocator);

        const system_t    ships_system   = ecs.register_system(gSystemShips, "system/ships");
        const component_t ship_pos_cp    = ecs.register_component<vector3_t>(ships_system, "position");
        const component_t ship_dir_cp    = ecs.register_component<vector3_t>(ships_system, "direction");
        const component_t ship_speed_cp  = ecs.register_component<f32>(ships_system, "speed");
        const component_t ship_radius_cp = ecs.register_component<f32>(ships_system, "radius");

        const entity_t bug1 = ecs.create_entity(ships_system, "entity/bug1");
        ecs.add_component<vector3_t>(bug1, ship_pos_cp, vector3_t::zero); // by id
        ecs.add_component<vector3_t>(bug1, ship_dir_cp, vector3_t::up);   // by id
        ecs.add_component<f32>(bug1, "speed", 10.0f);                     // by name
        ecs.add_component<f32>(bug1, "radius", 1.0f);                     // by name

        const system_t ai_system     = ecs.register_system(gSystemAI, "system/ai");
        const system_t vfx_system    = ecs.register_system(gSystemGfxVfx, "system/gfx/vfx");
        const system_t sfx_system    = ecs.register_system(gSystemAudioSfx, "system/audio/sfx");
        const system_t render_system = ecs.register_system(gSystemRender, "system/gfx/render");

        msg_system_t& msg = *ecs.msg;

        const id_t explosion_msg_id = ecs.register_id("msg/explosion");

        const property_t pos_prop    = msg.register_property<vector3_t>("position", vector3_t::zero);
        const property_t radius_prop = msg.register_property<float>("radius", 10.0f);

        // what about composing a message using named (registered) properties?
        vector3_t explosion_pos = {100.0f, 2.0f, 5.0f};

        // position, radius, damage
        const msg_t explosion_msg = msg.begin(explosion_msg_id);
        {
            // properties by id
            //msg.write_property<vector3_t>(explosion_msg, pos_prop, explosion_pos); // by id
            //msg.write_property<f32>(explosion_msg, radius_prop, 10.0f);            // by id
            //msg.write_property<f32>(explosion_msg, "damage", 0.9f);                // by name
            msg.write(explosion_msg, explosion_pos);
            msg.write(explosion_msg, radius_prop);
            msg.write(explosion_msg, radius_prop);
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

using namespace ncore;

UNITTEST_SUITE_BEGIN(cmsg)
{
    UNITTEST_FIXTURE(url)
    {
        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

        UNITTEST_TEST(test) {}
    }
}
UNITTEST_SUITE_END
