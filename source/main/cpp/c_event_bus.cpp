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
        struct event_block_t
        {
            u32            m_event_size;
            u16            m_event_max;
            u16            m_event_count;
            event_block_t* m_next;
            event_block_t* m_prev;

            inline u8* base() { return (u8*)(this) + sizeof(event_block_t); }
            bool       is_full(u32 size) { return m_event_count >= m_event_max; }
            void*      alloc_event()
            {
                u16 const i = m_event_count++;
                return base() + (i * m_event_size);
            }
        };

        void* event_channel_t::alloc_event(u32 size)
        {
            if (m_event_block_head == nullptr || m_event_block_head->is_full(size))
            {
                event_block_t* block = (event_block_t*)m_event_allocator->allocate(sizeof(event_block_t) + (s_max_event_count * size));
                block->m_event_size  = (size + (8 - 1)) & ~(8 - 1);
                block->m_event_max   = s_max_event_count;
                block->m_event_count = 0;
                if (m_event_block_head != nullptr)
                {
                    block->m_prev                      = m_event_block_head->m_prev;
                    block->m_next                      = m_event_block_head;
                    m_event_block_head->m_prev->m_next = block;
                    m_event_block_head->m_prev         = block;
                }
                else
                {
                    block->m_next      = m_event_block_head;
                    block->m_prev      = m_event_block_head;
                    m_event_block_head = block;
                }
            }

            return m_event_block_head->alloc_event();
        }

        void* event_channel_t::alloc_heap(u32 size) { return m_heap_allocator->allocate(size); }

        void event_channel_t::process_events()
        {
            if (m_event_block_head == nullptr)
                return;

            event_block_t* block = m_event_block_head;
            do
            {
                fire_events(block->base(), block->m_event_size, block->m_event_count);
                block = block->m_next;
            } while (block != m_event_block_head);

            m_event_block_head = nullptr;
        }

        void event_channel_t::teardown()
        {
            event_block_t* block = m_event_block_head;
            if (block == nullptr)
                return;

            do
            {
                event_block_t* next = block->m_next;
                m_event_allocator->deallocate(block);
                block = next;
            } while (block != m_event_block_head);
        }

        struct event_bus_t
        {
            alloc_buffer_t*  m_heap_allocator;
            alloc_buffer_t*  m_event_allocator;
            s32              m_channel_count;
            event_channel_t* m_channels[s_max_event_count];
        };

        event_bus_t* create_event_bus(alloc_t* alloc)
        {
            event_bus_t* bus = (event_bus_t*)alloc->allocate(sizeof(event_bus_t));

            bus->m_channel_count = 0;
            for (u32 i = 0; i < s_max_event_count; ++i)
                bus->m_channels[i] = nullptr;

            alloc_buffer_t* event_buffer = alloc->construct<alloc_buffer_t>();
            event_buffer->init((byte*)alloc->allocate(8 * 1024 * 1024), 8 * 1024 * 1024);
            bus->m_event_allocator = event_buffer;

            alloc_buffer_t* heap_buffer = alloc->construct<alloc_buffer_t>();
            heap_buffer->init((byte*)alloc->allocate(1 * 1024 * 1024), 1 * 1024 * 1024);
            bus->m_heap_allocator = heap_buffer;

            return bus;
        }

        void destroy_event_bus(alloc_t* alloc, event_bus_t* bus)
        {
            for (u32 i = 0; i < bus->m_channel_count; ++i)
            {
                if (bus->m_channels[i] != nullptr)
                {
                    bus->m_channels[i]->teardown();
                    alloc->deallocate(bus->m_channels[i]);
                }
            }
            alloc->deallocate(bus->m_heap_allocator->data());
            alloc->deallocate(bus->m_event_allocator->data());
            alloc->deallocate(bus->m_heap_allocator);
            alloc->deallocate(bus->m_event_allocator);
            alloc->deallocate(bus);
        }

        void set_event_channel(event_bus_t* bus, event_id_t event_id, event_channel_t* channel)
        {
            channel->m_heap_allocator   = bus->m_heap_allocator;
            channel->m_event_allocator  = bus->m_event_allocator;
            channel->m_event_block_head = nullptr;
            bus->m_channels[event_id]   = channel;
        }
        void* get_event_channel(event_bus_t* bus, event_id_t event_id) { return bus->m_channels[event_id]; }

        void* alloc_heap_memory(event_bus_t* bus, u32 size) { return bus->m_heap_allocator->allocate(size); }
        void* alloc_frame_memory(event_bus_t* bus, u32 size) { return bus->m_event_allocator->allocate(size); }

        void* alloc_event_channel(event_bus_t* bus, event_id_t event_id, u32 size)
        {
            u8* mem = (u8*)alloc_heap_memory(bus, size);
            for (u32 i = 0; i < size; ++i)
                mem[i] = 0;
            bus->m_channels[event_id] = (event_channel_t*)mem;;
            bus->m_channel_count      = (s32)event_id + 1;
            return mem;
        }

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