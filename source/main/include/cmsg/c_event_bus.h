#ifndef __CMSG_EVENT_BUS_H__
#define __CMSG_EVENT_BUS_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "crtti/c_type_info.h"

namespace ncore
{
    class alloc_t;

    namespace nrtti
    {
        typedef u16 type_id_t;
        struct type_info_t;
    } // namespace nrtti

    namespace nevent
    {
        /* Markdown

        ```cpp
        // store it in controller / singleton / std::shared_ptr whatever you want
        auto bus = create_event_bus(alloc);
        ```

        1. Define events

        ```cpp
        namespace event // optional namespace
        {
            struct Gold
            {
                int goldReceived = 0;
            };

            struct OK {}; // Simple event when user press "OK" button
        }
        ```

        2. Subscribe

        ```cpp
        // ...
        event_listener_t* listener = create_listener(bus, [](const event::Gold& event) // listen with lambda
                        {
                             std::cout << "I received gold: " << event.goldReceived << " 💰" << std::endl;
                        });

        // Hud layer will receive info about gold
        HudLayer* hudLayer = ...;
        hudLayer->listener = create_listener<event::Gold>(bus, std::bind(&HudLayer::onGoldReceived, hudLayer, std::placeholders::_1));

        ```

        3. Spread the news

        ```cpp
        //Inform listeners about event
        bus_post(bus, event::Gold{12}); // 1 way
        bus_post<event::Gold>(bus, {12}); // 2 way

        event::Gold myGold{12};
        bus_post(bus, myGold); // 3 way
        ```

        4. Process the events

        ```cpp
        bus_process(bus);
        ```

        */
        struct event_bus_t;

        event_bus_t* create_event_bus(alloc_t* alloc);
        void   destroy(alloc_t* alloc, event_bus_t* bus);

        // Use C++17 to register listeners for events (std::function), see the above documentation
        // this is an empty struct, the c++ implementation will be something like
        struct event_listener_t;
        event_listener_t* register_event_listener(event_bus_t* bus, nrtti::type_id_t id);

        // structs/classes used as messages need to be registered using nrtti so that we
        // can use the type_id_t (index) as the key for event registration.
        template <typename T> event_listener_t* register_event_listener(event_bus_t* bus)
        {
            nrtti::type_id_t id = nrtti::type_info_t::get_id<T>();
            return register_event_listener(bus, id);
        }

        template <typename T> void send_event(event_bus_t* bus, T const& event);

    } // namespace nevent
} // namespace ncore

#endif // __CMSG_MSG_H__
