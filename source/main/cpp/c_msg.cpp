#include "cbase/c_allocator.h"
#include "cbase/c_context.h"
#include "cbase/c_memory.h"

namespace ncore
{
    namespace cmsg
    {
        // so the idea is to easily send a message to:

        // id = name-id, instance-id, resource-id

        // - a game object by id
        // - a component by id
        // - a group by id

        // this includes certain short hand notations:
        // - . within the scope of this game object
        // - * within the scope of the parent
        // - # within the scope of a group
    

    } // namespace cmsg
} // namespace ncore