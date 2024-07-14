#ifndef __CMSG_EVENT_BUS_H__
#define __CMSG_EVENT_BUS_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "cmsg/c_delegate.h"

namespace ncore
{
    class alloc_t;

    namespace nevent
    {
        static const u32 s_max_event_count = 1024;

        typedef s32 event_id_t;
        static s32  s_generate_event_id()
        {
            static s32 s_event_id;
            return ++s_event_id;
        }

        template <typename T> struct EventTypeInfo
        {
            static event_id_t get_event_id()
            {
                static const s32 s_event_id = s_generate_event_id();
                return (event_id_t)s_event_id;
            };
        };

        struct event_block_t;

        class event_channel_t
        {
        public:
            virtual void*  alloc_event(u32 size);
            virtual void*  alloc_heap(u32 size);
            alloc_t*       m_heap_allocator;
            alloc_t*       m_event_allocator;
            event_block_t* m_event_block_head;

            virtual void process_events();
            virtual void teardown();

            virtual void fire_events(void const* event, u32 event_size, u32 event_count) = 0;
        };

        template <typename T> class event_channel_typed_t : public event_channel_t
        {
        public:
            struct delegate_node_t
            {
                ncore::Callback1<void, T const&> m_delegate;
                delegate_node_t*                 m_next;
                delegate_node_t*                 m_prev;
            };
            delegate_node_t* m_head{nullptr};

            void add_delegate(ncore::Callback1<void, T const&> delegate)
            {
                void*            delegate_node_mem = alloc_heap((u32)sizeof(delegate_node_t));
                delegate_node_t* node              = new (delegate_node_mem) delegate_node_t();
                node->m_delegate                   = delegate;
                if (m_head == nullptr)
                {
                    m_head       = node;
                    node->m_next = node;
                    node->m_prev = node;
                }
                else
                {
                    node->m_next           = m_head;
                    node->m_prev           = m_head->m_prev;
                    m_head->m_prev->m_next = node;
                    m_head->m_prev         = node;
                }
            }

            virtual void teardown()
            {
                delegate_node_t* node = m_head;
                while (node != nullptr)
                {
                    delegate_node_t* next = node->m_next;
                    m_heap_allocator->deallocate(node);
                    node = next;
                }
            }

            virtual void fire_events(void const* event, u32 event_size, u32 event_count) override
            {
                delegate_node_t const* node = m_head;
                while (node != nullptr)
                {
                    u8 const* event_data = static_cast<u8 const*>(event);
                    for (u32 i = 0; i < event_count; ++i)
                    {
                        node->m_delegate(*static_cast<T const*>(event_data));
                        event_data += event_size;
                    }
                    node = node->m_next;
                }
            }
        };

        struct event_bus_t;

        event_bus_t* create_event_bus(alloc_t* alloc);
        void         destroy_event_bus(alloc_t* alloc, event_bus_t* bus);
        void         set_event_channel(event_bus_t* bus, event_id_t event_id, event_channel_t* channel);
        void*        get_event_channel(event_bus_t* bus, event_id_t event_id);
        void*        alloc_heap_memory(event_bus_t* bus, u32 size);
        void*        alloc_frame_memory(event_bus_t* bus, u32 size);
        void         process_events(event_bus_t* bus);

        // structs/classes used as messages need to be registered using nrtti so that we
        // can use the type_id_t (index) as the key for event registration.
        template <typename T> void event_subscriber(event_bus_t* bus, ncore::Callback1<void, T const&> delegate)
        {
            event_id_t                id      = EventTypeInfo<T>::get_event_id();
            event_channel_typed_t<T>* channel = static_cast<event_channel_typed_t<T>*>(get_event_channel(bus, id));
            if (channel == nullptr)
            {
                void* channel_mem = alloc_heap_memory(bus, sizeof(T));
                channel           = new (new_signature(), channel_mem) event_channel_typed_t<T>();
                set_event_channel(bus, id, channel);
            }
            channel->add_delegate(delegate);
        }

        template <typename T> void post_event(event_bus_t* bus, T const& event)
        {
            event_id_t       id      = EventTypeInfo<T>::get_event_id();
            event_channel_t* channel = static_cast<event_channel_t*>(get_event_channel(bus, id));
            if (channel != nullptr)
            {
                void* event_mem = channel->alloc_event(sizeof(T));
                new (event_mem) T(event);
            }
        }

    } // namespace nevent
} // namespace ncore

#endif // __CMSG_MSG_H__
