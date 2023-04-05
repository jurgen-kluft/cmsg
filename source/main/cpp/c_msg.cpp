#include "cbase/c_allocator.h"
#include "cbase/c_context.h"
#include "cbase/c_memory.h"

namespace ncore
{
    namespace cmsg
    {
        // The whole idea of entity / component / system is to be able to
        // send messages to a specific entity or a specific component
        // or a specific system
        //
        // The message system leans heavily on the concept of 'frame' allocators
        // to avoid memory fragmentation and to allow for fast allocation and no deallocation.
        //

    
    } // namespace cmsg
} // namespace ncore