#ifndef __CMSG_EVENT_BUS_H__
#define __CMSG_EVENT_BUS_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ccore/c_callback.h"

namespace ncore
{
    class alloc_t;

    namespace nevent
    {
        typedef s32 event_id_t;

        struct event_bus_t;

        event_bus_t* create_event_bus(alloc_t* alloc, s32 max_channels, u32 event_memory_size, u32 heap_memory_size);
        void         destroy_event_bus(alloc_t* alloc, event_bus_t* bus);
        void         process_events(event_bus_t* bus);

        struct event_channel_t;
        void             set_event_channel(event_bus_t* bus, event_id_t event_id, event_channel_t* channel);
        event_channel_t* get_event_channel(event_bus_t* bus, event_id_t event_id);

        struct event_channel_t
        {
        public:
            void* alloc_event(u32 size);
            void* alloc_heap(u32 size);
            void  dealloc_heap(void* ptr);
            void  process_events();
            void  setup(event_bus_t* bus) { v_setup(bus); }
            void  teardown() { v_teardown(); }
            void  fire_events(void const* event, u32 event_size, u32 event_count) { v_fire_events(event, event_size, event_count); }

        protected:
            virtual void v_setup(event_bus_t* bus)                                         = 0;
            virtual void v_teardown()                                                      = 0;
            virtual void v_fire_events(void const* event, u32 event_size, u32 event_count) = 0;

            event_bus_t* m_bus;
            event_box_t* m_events;
        };

        template <typename T> class event_channel_typed_t : public event_channel_t
        {
        public:
            DCORE_CLASS_PLACEMENT_NEW_DELETE

            struct delegate_node_t
            {
                DCORE_CLASS_PLACEMENT_NEW_DELETE

                ncore::callback_t<void, T const&> m_delegate;
                delegate_node_t*                  m_next;
                delegate_node_t*                  m_prev;
            };
            delegate_node_t* m_head;

            void add_delegate(ncore::callback_t<void, T const&>& delegate)
            {
                // Check for duplicates
                delegate_node_t* node = m_head;
                while (node != nullptr)
                {
                    if (node->m_delegate == delegate)
                        return true;
                    node = node->m_next;
                };

                // Add delegate (unique)
                void* delegate_node_mem = alloc_heap((u32)sizeof(delegate_node_t));
                node                    = new (delegate_node_mem) delegate_node_t();
                node->m_delegate        = delegate;
                node->m_prev            = nullptr;
                node->m_next            = m_head;
                if (m_head != nullptr)
                    m_head->m_prev = node;
                m_head = node;
            }

            bool remove_delegate(ncore::callback_t<void, T const&> delegate)
            {
                delegate_node_t* node = m_head;
                while (node != nullptr)
                {
                    if (node->m_delegate == delegate)
                    {
                        if (node->m_prev != nullptr)
                            node->m_prev->m_next = node->m_next;
                        if (node->m_next != nullptr)
                            node->m_next->m_prev = node->m_prev;
                        if (m_head == node)
                            m_head = node->m_next;
                        dealloc_heap(node);
                        return true;
                    }
                    node = node->m_next;
                };
                return false;
            }

        protected:
            virtual void v_setup(bus_t* bus) override
            {
                m_bus  = bus;
                m_head = nullptr;
            }

            virtual void v_teardown() override
            {
                event_channel_t::teardown();
                if (m_head == nullptr)
                    return;

                delegate_node_t* node = m_head;
                while (node != nullptr)
                {
                    m_head = node->m_next;
                    dealloc_heap(node);
                    node = m_head;
                };
            }

            virtual void v_fire_events(void const* event, u32 event_size, u32 event_count) override
            {
                delegate_node_t const* node = m_head;
                while (node != nullptr)
                {
                    u8 const* event_data = static_cast<u8 const*>(event);
                    for (u32 i = 0; i < event_count; ++i)
                    {
                        node->m_delegate(*(T const*)event_data);
                        event_data += event_size;
                    }
                    node = node->m_next;
                };
            }
        };

        s32 new_global_event_id(event_bus_t* bus);

        // POD structs used as messages need to have a static member variable s_event_id initialized to -1
        template <typename T> void register_event_subscriber(event_bus_t* bus, ncore::callback_t<void, T const&> delegate)
        {
            if (T::s_event_id < 0)
                T::s_event_id = new_global_event_id(bus);

            event_channel_t* channel = get_event_channel(bus, T::s_event_id);
            if (channel == nullptr)
            {
                void* channel_mem = alloc_heap_memory(bus, (u32)sizeof(event_channel_typed_t<T>));
                clear_memory(channel_mem, (u32)sizeof(event_channel_typed_t<T>));
                channel = new (channel_mem) event_channel_typed_t<T>();
                set_event_channel(bus, id, channel);
            }
            channel->add_delegate(delegate);
        }

        template <typename T> void unregister_event_subscriber(event_bus_t* bus, ncore::callback_t<void, T const&> delegate)
        {
            if (T::s_event_id < 0)
                return; // Event type not registered

            event_channel_t* channel = get_event_channel(bus, T::s_event_id);
            if (channel == nullptr)
                return;
            event_channel_typed_t<T>* typed_channel = static_cast<event_channel_typed_t<T>*>(channel);
            typed_channel->remove_delegate(delegate);
        }

        template <typename T> void post_event(event_bus_t* bus, T const& event)
        {
            const event_id_t id      = T::s_event_id;
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
