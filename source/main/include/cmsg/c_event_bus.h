#ifndef __CMSG_EVENT_BUS_H__
#define __CMSG_EVENT_BUS_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

namespace ncore
{
    class alloc_t;

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
        listener_t* listener = create_listener(bus, [](const event::Gold& event) // listen with lambda
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
        struct bus_t;

        bus_t* create_event_bus(alloc_t* alloc);
        void   destroy(alloc_t* alloc, bus_t* bus);

        // Use C++17 to register listeners for events (std::function), see the above documentation
        // this is an empty struct, the c++ implementation will be something like
        struct listener_t;

        listener_t* register_listener(bus_t* bus);

    } // namespace nevent
} // namespace ncore

#endif // __CMSG_MSG_H__
