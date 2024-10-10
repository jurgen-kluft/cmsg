#include "cbase/c_allocator.h"
#include "ccore/c_debug.h"
#include "cbase/c_context.h"
#include "cbase/c_debug.h"
#include "cbase/c_memory.h"

#include "cmsg/c_event_bus.h"

namespace ncore
{
    namespace nevent
    {
        struct event_box_t;

        static const s32 s_max_event_count = 1024;

        struct event_bus_t;

        void* alloc_heap_memory(event_bus_t* bus, u32 size);
        void* alloc_frame_memory(event_bus_t* bus, u32 size);
        void  clear_memory(void* mem, u32 size);
        void  process_events(event_bus_t* bus);

        struct event_box_t
        {
            u32          m_size;
            u16          m_max;
            u16          m_count;
            event_box_t* m_next;

            inline u8* base() { return (u8*)(this) + sizeof(event_box_t); }
            bool       is_full(u32 size) { return m_count >= m_max; }
            void*      alloc_event()
            {
                u16 const i = m_count++;
                return base() + (i * m_size);
            }
        };

        void* event_channel_t::alloc_event(u32 size)
        {
            if (m_events == nullptr || m_events->is_full(size))
            {
                u32 const    event_size = (size + (8 - 1)) & ~(8 - 1);
                event_box_t* block      = (event_box_t*)m_bus->m_event_allocator->allocate(sizeof(event_box_t) + (s_max_event_count * event_size));
                block->m_size           = event_size;
                block->m_max            = s_max_event_count;
                block->m_count          = 0;
                block->m_next           = m_events;
                m_events                = block;
            }

            return m_events->alloc_event();
        }

        void* event_channel_t::alloc_heap(u32 size) { return m_bus->m_heap_allocator->allocate(size); }
        void  event_channel_t::dealloc_heap(void* ptr) { m_bus->m_heap_allocator->deallocate(ptr); }

        void event_channel_t::process_events()
        {
            event_box_t* box = m_events;
            while (box != nullptr)
            {
                fire_events(box->base(), box->m_size, box->m_count);
                box = box->m_next;
            };

            m_events = nullptr;
        }

        struct event_bus_t
        {
            alloc_buffer_t*   m_heap_allocator;
            alloc_buffer_t*   m_event_allocator;
            s32               m_channel_count;
            s32               m_channel_max;
            event_channel_t** m_channels;
            s32               m_global_event_id;
        };

        s32 new_global_event_id(event_bus_t* bus) { return bus->m_global_event_id++; }

        event_bus_t* create_event_bus(alloc_t* alloc, s32 max_channels, u32 event_memory_size, u32 heap_memory_size)
        {
            event_bus_t* bus       = (event_bus_t*)alloc->allocate(sizeof(event_bus_t));
            bus->m_global_event_id = 0;
            bus->m_channel_max     = max_channels;
            bus->m_channel_count   = 0;
            for (u32 i = 0; i < max_channels; ++i)
                bus->m_channels[i] = nullptr;

            alloc_buffer_t* event_buffer = alloc->construct<alloc_buffer_t>();
            event_buffer->init((byte*)alloc->allocate(event_memory_size), event_memory_size);
            bus->m_event_allocator = event_buffer;

            alloc_buffer_t* heap_buffer = alloc->construct<alloc_buffer_t>();
            heap_buffer->init((byte*)alloc->allocate(heap_memory_size), heap_memory_size);
            bus->m_heap_allocator = heap_buffer;

            return bus;
        }

        void destroy_event_bus(alloc_t* alloc, event_bus_t* bus)
        {
            alloc->deallocate(bus->m_heap_allocator->data());
            alloc->deallocate(bus->m_event_allocator->data());
            alloc->deallocate(bus->m_heap_allocator);
            alloc->deallocate(bus->m_event_allocator);
            alloc->deallocate(bus);
        }

        void set_event_channel(event_bus_t* bus, event_id_t event_id, event_channel_t* channel)
        {
            channel->setup(bus);
            bus->m_channels[event_id] = channel;
            bus->m_channel_count      = (s32)event_id + 1;
        }

        event_channel_t* get_event_channel(event_bus_t* bus, event_id_t event_id) { return bus->m_channels[event_id]; }

        void clear_memory(void* ptr, u32 size)
        {
            u8* mem = (u8*)ptr;
            for (u32 i = 0; i < size; ++i)
                mem[i] = 0;
        }

        void* alloc_heap_memory(event_bus_t* bus, u32 size) { return bus->m_heap_allocator->allocate(size); }
        void* alloc_frame_memory(event_bus_t* bus, u32 size) { return bus->m_event_allocator->allocate(size); }

        void process_events(event_bus_t* bus)
        {
            s32 const n = bus->m_channel_count;
            for (s32 i = 0; i < n; ++i)
            {
                event_channel_t* channel = bus->m_channels[i];
                if (channel != nullptr)
                    channel->process_events();
            }
        }

    } // namespace nevent
} // namespace ncore
