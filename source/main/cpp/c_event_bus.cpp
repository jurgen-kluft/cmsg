#include "ccore/c_allocator.h"
#include "ccore/c_debug.h"
#include "cbase/c_context.h"
#include "cbase/c_debug.h"
#include "cbase/c_memory.h"
#include "cbase/c_hbb.h"
#include "cbase/c_runes.h"

#include "cmsg/c_event_bus.h"

namespace ncore
{
    namespace nevent
    {
        const s32 MAX_EVENT_TYPES = 64;

        /*
        Events/Messages are copied into a single memory buffer.
         */

        struct event_t
        {
            event_t* m_next;
            u32      m_size;
        };

        struct event_channel_t
        {
            nrtti::type_id_t m_type_id;
            event_t*         m_events;
        };

        struct event_bus_t
        {
            nrtti::type_id_t  m_type_id[MAX_EVENT_TYPES];
            event_listener_t* m_listeners[MAX_EVENT_TYPES];
        };

        event_listener_t* register_event_listener(event_bus_t* bus, const nrtti::type_id_t& id) { return nullptr; }

    } // namespace nevent
} // namespace ncore