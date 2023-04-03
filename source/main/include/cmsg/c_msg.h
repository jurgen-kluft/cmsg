#ifndef __CMSG_MSG_H__
#define __CMSG_MSG_H__
#include "cbase/c_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

#include "cbase/c_debug.h"

namespace ncore
{
    namespace cmsg
    {
        template <typename T>
        post(gMsgSystem, const char * system, const char * path, const char * identity, T const& msg);

        /*
        vector3_t pos(0, 0, 0);

        // what about composing a message using named (registered) properties?
        explosion_msg_t msg(pos, 10.0f, 150);

        cmsg::post(gMsgSystem, "go", "ships", "enemy", msg);
        cmsg::post(gMsgSystem, "gfx", "vfx", "explosion", explosion_msg_t(pos, 10.0f, 150));
        cmsg::post(gMsgSystem, "audio", "sfx", "explosion", explosion_msg_t(pos, 10.0f, 150));
        */

    } // namespace cmsg
} // namespace ncore

#endif // __CMSG_MSG_H__