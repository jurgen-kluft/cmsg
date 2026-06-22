#include "ccore/c_allocator.h"
#include "ccore/c_debug.h"

#include "cmsg/c_msg.h"

namespace ncore
{
    namespace nmsg
    {
        //
        // .______   .______        ______   .___________.  ______   .___________.____    ____ .______    _______
        // |   _  \  |   _  \      /  __  \  |           | /  __  \  |           |\   \  /   / |   _  \  |   ____|
        // |  |_)  | |  |_)  |    |  |  |  | `---|  |----`|  |  |  | `---|  |----` \   \/   /  |  |_)  | |  |__
        // |   ___/  |      /     |  |  |  |     |  |     |  |  |  |     |  |       \_    _/   |   ___/  |   __|
        // |  |      |  |\  \----.|  `--'  |     |  |     |  `--'  |     |  |         |  |     |  |      |  |____
        // | _|      | _| `._____| \______/      |__|      \______/      |__|         |__|     | _|      |_______|
        //

        // The whole idea of entity / component / system is to be able to
        // send messages to a specific entity or a specific component
        // or a specific system
        //
        // The message system leans heavily on the concept of 'frame' allocators
        // to avoid memory fragmentation and to allow for fast allocation and
        // batch based deallocation.
        //

        // the defaults of the above types are:
        template <> const u8   type_t<u8>::default_value   = 0;
        template <> const u16  type_t<u16>::default_value  = 0;
        template <> const u32  type_t<u32>::default_value  = 0;
        template <> const u64  type_t<u64>::default_value  = 0;
        template <> const s8   type_t<s8>::default_value   = 0;
        template <> const s16  type_t<s16>::default_value  = 0;
        template <> const s32  type_t<s32>::default_value  = 0;
        template <> const s64  type_t<s64>::default_value  = 0;
        template <> const f32  type_t<f32>::default_value  = 0.0f;
        template <> const f64  type_t<f64>::default_value  = 0.0f;
        template <> const bool type_t<bool>::default_value = false;

        template <> typeinfo_t type_t<u8>::typeinfo("u8", &type_t<u8>::default_value, (u32)sizeof(u8));
        template <> typeinfo_t type_t<u16>::typeinfo("u16", &type_t<u16>::default_value, sizeof(u16));
        template <> typeinfo_t type_t<u32>::typeinfo("u32", &type_t<u32>::default_value, sizeof(u32));
        template <> typeinfo_t type_t<u64>::typeinfo("u64", &type_t<u64>::default_value, sizeof(u64));
        template <> typeinfo_t type_t<s8>::typeinfo("s8", &type_t<s8>::default_value, sizeof(s8));
        template <> typeinfo_t type_t<s16>::typeinfo("s16", &type_t<s16>::default_value, sizeof(s16));
        template <> typeinfo_t type_t<s32>::typeinfo("s32", &type_t<s32>::default_value, sizeof(s32));
        template <> typeinfo_t type_t<s64>::typeinfo("s64", &type_t<s64>::default_value, sizeof(s64));
        template <> typeinfo_t type_t<f32>::typeinfo("f32", &type_t<f32>::default_value, sizeof(f32));
        template <> typeinfo_t type_t<f64>::typeinfo("f64", &type_t<f64>::default_value, sizeof(f64));
        template <> typeinfo_t type_t<bool>::typeinfo("bool", &type_t<bool>::default_value, sizeof(bool));

        // ------------------------------------------------------------------------------------------------------------------------
        // ------------------------------------------------------------------------------------------------------------------------
        // ------------------------------------------------------------------------------------------------------------------------
        // ------------------------------------------------------------------------------------------------------------------------

        static void use_case()
        {
            // ...
        }

    } // namespace nmsg
} // namespace ncore
