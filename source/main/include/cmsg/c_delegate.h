#ifndef __CMSG_CPP_DELEGATE_H__
#define __CMSG_CPP_DELEGATE_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ccore/c_debug.h"
#include "ccore/c_allocator.h"

namespace ncore
{
    namespace __private__
    {
        inline s32 compare_mem(const void* lhs, const void* rhs, u32 size)
        {
            u8 const* l = (u8 const*)lhs;
            u8 const* r = (u8 const*)rhs;
            for (u32 i = 0; i < size; ++i)
            {
                if (l[i] < r[i])
                    return -1;
                if (l[i] > r[i])
                    return 1;
            }
            return 0;
        }

        inline void copy_mem(void* dst, const void* src, u32 size)
        {
            u8*       d = (u8*)dst;
            u8 const* s = (u8 const*)src;
            for (u32 i = 0; i < size; ++i)
                d[i] = s[i];
        }
    } // namespace __private__

    /// Stores a callback for a function taking 0 parameters.
    ///\tparam R Callback function return type.
    template <typename R> class Callback0
    {
    public:
        /// Constructs the callback to a specific object and member function.
        ///\param object Pointer to the object to call upon. Care should be taken that this object remains valid as long as the callback may be invoked.
        ///\param function Member function address to call.
        template <typename C>
        Callback0(C* object, R (C::*function)())
            : mCallback(new(new_signature(), &mMem) ChildMethod<C>(object, function))
        {
        }

        /// Constructs the callback to a free function or static member function.
        ///\param function Free function address to call.
        Callback0(R (*function)())
            : mCallback(new(new_signature(), &mMem) ChildFree(function))
        {
        }

        /// Constructs a callback that can later be set.
        Callback0()
            : mCallback(0)
        {
        }

        Callback0(const Callback0& c)
            : mCallback(c.mCallback)
        {
            if (mCallback)
            {
                __private__::copy_mem(mMem, c.mMem, (u32)sizeof(mMem));
                mCallback = reinterpret_cast<Base*>(&mMem);
            }
        }

        Callback0& operator=(const Callback0& rhs)
        {
            mCallback = rhs.mCallback;
            if (mCallback)
            {
                __private__::copy_mem(mMem, rhs.mMem, (u32)sizeof(mMem));
                mCallback = reinterpret_cast<Base*>(&mMem);
            }

            return *this;
        }

        ~Callback0() {}

        /// Sets the callback to a specific object and member function.
        ///\param object Pointer to the object to call upon. Care should be taken that this object remains valid as long as the callback may be invoked.
        ///\param function Member function address to call.
        template <typename C> void Reset(C* object, R (C::*function)()) { mCallback = new (&mMem) ChildMethod<C>(object, function); }

        /// Sets the callback to a free function or static member function.
        ///\param function Free function address to call.
        void Reset(R (*function)()) { mCallback = new (&mMem) ChildFree(function); }

        /// Resests to callback to nothing.
        void Reset() { mCallback = 0; }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator==(const Callback0& rhs) const
        {
            if (mCallback && rhs.mCallback)
                return (*mCallback) == (*(rhs.mCallback));
            return mCallback == rhs.mCallback;
        }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator!=(const Callback0& rhs) const { return !(*this == rhs); }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator<(const Callback0 rhs) const
        {
            if (mCallback && rhs.mCallback)
                return (*mCallback) < (*(rhs.mCallback));
            return mCallback < rhs.mCallback;
        }

        /// Returns true if the callback has been set, or false if the callback is not set and is invalid.
        bool IsSet() const { return mCallback; }

        /// Invokes the callback.
        R operator()() const
        {
            if (mCallback)
                return (*mCallback)();
            return R();
        }

        /// Invokes the callback. This function can sometimes be more convenient than the operator(), which does the same thing.
        R Call() const
        {
            if (mCallback)
                return (*mCallback)();
            return R();
        }

    private:
        class Base
        {
        public:
            Base() {}
            virtual R           operator()()                      = 0;
            virtual bool        operator==(const Base& rhs) const = 0;
            virtual bool        operator<(const Base& rhs) const  = 0;
            virtual void const* FreeFunction() const              = 0;
            virtual void const* MethodFunction() const            = 0;
            virtual void*       Comp() const                      = 0; // Returns a pointer used in comparisons.
        };

        class ChildFree : public Base
        {
        public:
            ChildFree(R (*function)())
                : mFunc(function)
            {
            }

            virtual R operator()() { return mFunc(); }

            virtual bool operator==(const Base& rhs) const
            {
                const ChildFree* const r = (const ChildFree*)rhs.FreeFunction();
                if (!r)
                    return false;
                return (mFunc == r->mFunc);
            }

            virtual bool operator<(const Base& rhs) const
            {
                const ChildFree* const r = (const ChildFree*)rhs.FreeFunction();
                if (!r)
                    return true; // Free functions will always be less than methods (because comp returns 0).
                return mFunc < r->mFunc;
            }

            virtual void const* FreeFunction() const { return this; }
            virtual void const* MethodFunction() const { return nullptr; }

            virtual void* Comp() const { return 0; }

        private:
            R (*const mFunc)();
        };

        template <typename C> class ChildMethod : public Base
        {
        public:
            ChildMethod(C* object, R (C::*function)())
                : mObj(object)
                , mFunc(function)
            {
            }

            virtual R operator()() { return (mObj->*mFunc)(); }

            virtual bool operator==(const Base& rhs) const
            {
                const ChildMethod<C>* const r = (const ChildMethod<C>*)rhs.MethodFunction();
                if (!r)
                    return false;
                return (mObj == r->mObj) && (mFunc == r->mFunc);
            }

            virtual bool operator<(const Base& rhs) const
            {
                const ChildMethod<C>* const r = (const ChildMethod<C>*)rhs.MethodFunction();
                if (!r)
                    return mObj < rhs.Comp();

                if (mObj != r->mObj)
                    return mObj < r->mObj;
                return 0 > __private__::compare_mem((void const*)&mFunc, (void*)&(r->mFunc), (u32)sizeof(mFunc));
            }

            virtual void const* FreeFunction() const { return nullptr; }
            virtual void const* MethodFunction() const { return this; }

            virtual void* Comp() const { return mObj; }

        private:
            C* const mObj;
            R (C::*const mFunc)();
        };

        /// This class is only to find the worst case method pointer size.
        class unknown;

        char  mMem[sizeof(ChildMethod<unknown>)]; // Reserve memory for creating useful objects later.
        Base* mCallback;
    };

    /// Helper function to construct a callback without bothering to specify template parameters.
    template <typename C, typename R> Callback0<R> Make0(C* object, R (C::*function)()) { return Callback0<R>(object, function); }

    /// Helper function to construct a callback without bothering to specify template parameters.
    template <typename R> Callback0<R> Make0(R (*function)()) { return Callback0<R>(function); }

    /// Stores a callback for a function taking 1 parameters.
    ///\tparam R Callback function return type.
    template <typename R, typename T0> class Callback1
    {
    public:
        /// Constructs the callback to a specific object and member function.
        ///\param object Pointer to the object to call upon. Care should be taken that this object remains valid as long as the callback may be invoked.
        ///\param function Member function address to call.
        template <typename C>
        Callback1(C* object, R (C::*function)(T0 t0))
            : mCallback(new(new_signature(), &mMem) ChildMethod<C>(object, function))
        {
        }

        /// Constructs the callback to a free function or static member function.
        ///\param function Free function address to call.
        Callback1(R (*function)(T0 t0))
            : mCallback(new(new_signature(), &mMem) ChildFree(function))
        {
        }

        /// Constructs a callback that can later be set.
        Callback1()
            : mCallback(0)
        {
        }

        Callback1(const Callback1& c)
            : mCallback(c.mCallback)
        {
            if (mCallback)
            {
                __private__::copy_mem(mMem, c.mMem, (u32)sizeof(mMem));
                mCallback = reinterpret_cast<Base*>(&mMem);
            }
        }

        Callback1& operator=(const Callback1& rhs)
        {
            mCallback = rhs.mCallback;
            if (mCallback)
            {
                __private__::copy_mem(mMem, rhs.mMem, (u32)sizeof(mMem));
                mCallback = reinterpret_cast<Base*>(&mMem);
            }

            return *this;
        }

        ~Callback1() {}

        /// Sets the callback to a specific object and member function.
        ///\param object Pointer to the object to call upon. Care should be taken that this object remains valid as long as the callback may be invoked.
        ///\param function Member function address to call.
        template <typename C> void Reset(C* object, R (C::*function)(T0 t0)) { mCallback = new (new_signature(), &mMem) ChildMethod<C>(object, function); }

        /// Sets the callback to a free function or static member function.
        ///\param function Free function address to call.
        void Reset(R (*function)(T0 t0)) { mCallback = new (&mMem) ChildFree(function); }

        /// Resests to callback to nothing.
        void Reset() { mCallback = 0; }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator==(const Callback1& rhs) const
        {
            if (mCallback && rhs.mCallback)
                return (*mCallback) == (*(rhs.mCallback));
            return mCallback == rhs.mCallback;
        }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator!=(const Callback1& rhs) const { return !(*this == rhs); }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator<(const Callback1 rhs) const
        {
            if (mCallback && rhs.mCallback)
                return (*mCallback) < (*(rhs.mCallback));
            return mCallback < rhs.mCallback;
        }

        /// Returns true if the callback has been set, or false if the callback is not set and is invalid.
        bool IsSet() const { return mCallback; }

        /// Invokes the callback.
        R operator()(T0 t0) const
        {
            if (mCallback)
                return (*mCallback)(t0);
            return R();
        }

        /// Invokes the callback. This function can sometimes be more convenient than the operator(), which does the same thing.
        R Call(T0 t0) const
        {
            if (mCallback)
                return (*mCallback)(t0);
            return R();
        }

    private:
        class Base
        {
        public:
            Base() {}
            virtual R           operator()(T0 t0)                 = 0;
            virtual bool        operator==(const Base& rhs) const = 0;
            virtual bool        operator<(const Base& rhs) const  = 0;
            virtual void const* FreeFunction() const              = 0;
            virtual void const* MethodFunction() const            = 0;
            virtual void*       Comp() const                      = 0; // Returns a pointer used in comparisons.
        };

        class ChildFree : public Base
        {
        public:
            ChildFree(R (*function)(T0 t0))
                : mFunc(function)
            {
            }

            virtual R operator()(T0 t0) { return mFunc(t0); }

            virtual bool operator==(const Base& rhs) const
            {
                const ChildFree* const r = (const ChildFree*)rhs.FreeFunction();
                if (r)
                    return (mFunc == r->mFunc);
                return false;
            }

            virtual bool operator<(const Base& rhs) const
            {
                const ChildFree* const r = (const ChildFree*)rhs.FreeFunction();
                if (r)
                    return (void const*)mFunc < (void const*)r->mFunc;
                return true; // Free functions will always be less than methods (because comp returns 0).
            }

            virtual void const* FreeFunction() const { return this; }
            virtual void const* MethodFunction() const { return nullptr; }

            virtual void* Comp() const { return 0; }

        private:
            R (*const mFunc)(T0 t0);
        };

        template <typename C> class ChildMethod : public Base
        {
        public:
            ChildMethod(C* object, R (C::*function)(T0 t0))
                : mObj(object)
                , mFunc(function)
            {
            }

            virtual R operator()(T0 t0) { return (mObj->*mFunc)(t0); }

            virtual bool operator==(const Base& rhs) const
            {
                const ChildMethod<C>* const r = (const ChildMethod<C>*)rhs.MethodFunction();
                if (r)
                    return (mObj == r->mObj) && (mFunc == r->mFunc);
                return false;
            }

            virtual bool operator<(const Base& rhs) const
            {
                const ChildMethod<C>* const r = (const ChildMethod<C>*)rhs.MethodFunction();
                if (r)
                {
                    if (mObj != r->mObj)
                        return mObj < r->mObj;
                    return 0 > __private__::compare_mem((void const*)&mFunc, (void*)&(r->mFunc), (u32)sizeof(mFunc));
                }
                return mObj < rhs.Comp();
            }
            virtual void const* FreeFunction() const { return nullptr; }
            virtual void const* MethodFunction() const { return this; }

            virtual void* Comp() const { return mObj; }

        private:
            C* const mObj;
            R (C::*const mFunc)(T0 t0);
        };

        /// This class is only to find the worst case method pointer size.
        class unknown;

        char  mMem[sizeof(ChildMethod<unknown>)]; // Reserve memory for creating useful objects later.
        Base* mCallback;
    };

    /// Helper function to construct a callback without bothering to specify template parameters.
    template <typename C, typename R, typename T0> Callback1<R, T0> Make1(C* object, R (C::*function)(T0 t0)) { return Callback1<R, T0>(object, function); }

    /// Helper function to construct a callback without bothering to specify template parameters.
    template <typename R, typename T0> Callback1<R, T0> Make1(R (*function)(T0 t0)) { return Callback1<R, T0>(function); }

    /// Stores a callback for a function taking 2 parameters.
    ///\tparam R Callback function return type.
    template <typename R, typename T0, typename T1> class Callback2
    {
    public:
        /// Constructs the callback to a specific object and member function.
        ///\param object Pointer to the object to call upon. Care should be taken that this object remains valid as long as the callback may be invoked.
        ///\param function Member function address to call.
        template <typename C>
        Callback2(C* object, R (C::*function)(T0 t0, T1 t1))
            : mCallback(new(new_signature(), &mMem) ChildMethod<C>(object, function))
        {
        }

        /// Constructs the callback to a free function or static member function.
        ///\param function Free function address to call.
        Callback2(R (*function)(T0 t0, T1 t1))
            : mCallback(new(new_signature(), &mMem) ChildFree(function))
        {
        }

        /// Constructs a callback that can later be set.
        Callback2()
            : mCallback(0)
        {
        }

        Callback2(const Callback2& c)
            : mCallback(c.mCallback)
        {
            if (mCallback)
            {
                __private__::copy_mem(mMem, c.mMem, (u32)sizeof(mMem));
                mCallback = reinterpret_cast<Base*>(&mMem);
            }
        }

        Callback2& operator=(const Callback2& rhs)
        {
            mCallback = rhs.mCallback;
            if (mCallback)
            {
                __private__::copy_mem(mMem, rhs.mMem, (u32)sizeof(mMem));
                mCallback = reinterpret_cast<Base*>(&mMem);
            }

            return *this;
        }

        ~Callback2() {}

        /// Sets the callback to a specific object and member function.
        ///\param object Pointer to the object to call upon. Care should be taken that this object remains valid as long as the callback may be invoked.
        ///\param function Member function address to call.
        template <typename C> void Reset(C* object, R (C::*function)(T0 t0, T1 t1)) { mCallback = new (&mMem) ChildMethod<C>(object, function); }

        /// Sets the callback to a free function or static member function.
        ///\param function Free function address to call.
        void Reset(R (*function)(T0 t0, T1 t1)) { mCallback = new (&mMem) ChildFree(function); }

        /// Resests to callback to nothing.
        void Reset() { mCallback = 0; }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator==(const Callback2& rhs) const
        {
            if (mCallback && rhs.mCallback)
                return (*mCallback) == (*(rhs.mCallback));
            else
                return mCallback == rhs.mCallback;
        }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator!=(const Callback2& rhs) const { return !(*this == rhs); }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator<(const Callback2 rhs) const
        {
            if (mCallback && rhs.mCallback)
                return (*mCallback) < (*(rhs.mCallback));
            else
                return mCallback < rhs.mCallback;
        }

        /// Returns true if the callback has been set, or false if the callback is not set and is invalid.
        bool IsSet() const { return mCallback; }

        /// Invokes the callback.
        R operator()(T0 t0, T1 t1) const
        {
            if (mCallback)
                return (*mCallback)(t0, t1);
            return R();
        }

        /// Invokes the callback. This function can sometimes be more convenient than the operator(), which does the same thing.
        R Call(T0 t0, T1 t1) const
        {
            if (mCallback)
                return (*mCallback)(t0, t1);
            return R();
        }

    private:
        class Base
        {
        public:
            Base() {}
            virtual R           operator()(T0 t0, T1 t1)          = 0;
            virtual bool        operator==(const Base& rhs) const = 0;
            virtual bool        operator<(const Base& rhs) const  = 0;
            virtual void const* FreeFunction() const              = 0;
            virtual void const* MethodFunction() const            = 0;
            virtual void*       Comp() const                      = 0; // Returns a pointer used in comparisons.
        };

        class ChildFree : public Base
        {
        public:
            ChildFree(R (*function)(T0 t0, T1 t1))
                : mFunc(function)
            {
            }

            virtual R operator()(T0 t0, T1 t1) { return mFunc(t0, t1); }

            virtual bool operator==(const Base& rhs) const
            {
                const ChildFree* const r = (const ChildFree*)rhs.FreeFunction();
                if (r)
                    return (mFunc == r->mFunc);
                else
                    return false;
            }

            virtual bool operator<(const Base& rhs) const
            {
                const ChildFree* const r = (const ChildFree*)rhs.FreeFunction();
                if (r)
                    return mFunc < r->mFunc;
                else
                    return true; // Free functions will always be less than methods (because comp returns 0).
            }

            virtual void const* FreeFunction() const { return this; }
            virtual void const* MethodFunction() const { return nullptr; }

            virtual void* Comp() const { return 0; }

        private:
            R (*const mFunc)(T0 t0, T1 t1);
        };

        template <typename C> class ChildMethod : public Base
        {
        public:
            ChildMethod(C* object, R (C::*function)(T0 t0, T1 t1))
                : mObj(object)
                , mFunc(function)
            {
            }

            virtual R operator()(T0 t0, T1 t1) { return (mObj->*mFunc)(t0, t1); }

            virtual bool operator==(const Base& rhs) const
            {
                const ChildMethod<C>* const r = (const ChildMethod<C>*)rhs.MethodFunction();
                if (r)
                    return (mObj == r->mObj) && (mFunc == r->mFunc);
                else
                    return false;
            }

            virtual bool operator<(const Base& rhs) const
            {
                const ChildMethod<C>* const r = (const ChildMethod<C>*)rhs.MethodFunction();
                if (r)
                {
                    if (mObj != r->mObj)
                        return mObj < r->mObj;
                    else
                        return 0 > __private__::compare_mem((void const*)&mFunc, (void*)&(r->mFunc), (u32)sizeof(mFunc));
                }
                else
                    return mObj < rhs.Comp();
            }
            virtual void const* FreeFunction() const { return nullptr; }
            virtual void const* MethodFunction() const { return this; }

            virtual void* Comp() const { return mObj; }

        private:
            C* const mObj;
            R (C::*const mFunc)(T0 t0, T1 t1);
        };

        /// This class is only to find the worst case method pointer size.
        class unknown;

        char  mMem[sizeof(ChildMethod<unknown>)]; // Reserve memory for creating useful objects later.
        Base* mCallback;
    };

    /// Helper function to construct a callback without bothering to specify template parameters.
    template <typename C, typename R, typename T0, typename T1> Callback2<R, T0, T1> Make2(C* object, R (C::*function)(T0 t0, T1 t1)) { return Callback2<R, T0, T1>(object, function); }

    /// Helper function to construct a callback without bothering to specify template parameters.
    template <typename R, typename T0, typename T1> Callback2<R, T0, T1> Make2(R (*function)(T0 t0, T1 t1)) { return Callback2<R, T0, T1>(function); }

    /// Stores a callback for a function taking 3 parameters.
    ///\tparam R Callback function return type.
    template <typename R, typename T0, typename T1, typename T2> class Callback3
    {
    public:
        /// Constructs the callback to a specific object and member function.
        ///\param object Pointer to the object to call upon. Care should be taken that this object remains valid as long as the callback may be invoked.
        ///\param function Member function address to call.
        template <typename C>
        Callback3(C* object, R (C::*function)(T0 t0, T1 t1, T2 t2))
            : mCallback(new(new_signature(), &mMem) ChildMethod<C>(object, function))
        {
        }

        /// Constructs the callback to a free function or static member function.
        ///\param function Free function address to call.
        Callback3(R (*function)(T0 t0, T1 t1, T2 t2))
            : mCallback(new(new_signature(), &mMem) ChildFree(function))
        {
        }

        /// Constructs a callback that can later be set.
        Callback3()
            : mCallback(0)
        {
        }

        Callback3(const Callback3& c)
            : mCallback(c.mCallback)
        {
            if (mCallback)
            {
                __private__::copy_mem(mMem, c.mMem, (u32)sizeof(mMem));
                mCallback = reinterpret_cast<Base*>(&mMem);
            }
        }

        Callback3& operator=(const Callback3& rhs)
        {
            mCallback = rhs.mCallback;
            if (mCallback)
            {
                __private__::copy_mem(mMem, rhs.mMem, (u32)sizeof(mMem));
                mCallback = reinterpret_cast<Base*>(&mMem);
            }

            return *this;
        }

        ~Callback3() {}

        /// Sets the callback to a specific object and member function.
        ///\param object Pointer to the object to call upon. Care should be taken that this object remains valid as long as the callback may be invoked.
        ///\param function Member function address to call.
        template <typename C> void Reset(C* object, R (C::*function)(T0 t0, T1 t1, T2 t2)) { mCallback = new (&mMem) ChildMethod<C>(object, function); }

        /// Sets the callback to a free function or static member function.
        ///\param function Free function address to call.
        void Reset(R (*function)(T0 t0, T1 t1, T2 t2)) { mCallback = new (&mMem) ChildFree(function); }

        /// Resests to callback to nothing.
        void Reset() { mCallback = 0; }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator==(const Callback3& rhs) const
        {
            if (mCallback && rhs.mCallback)
                return (*mCallback) == (*(rhs.mCallback));
            else
                return mCallback == rhs.mCallback;
        }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator!=(const Callback3& rhs) const { return !(*this == rhs); }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator<(const Callback3 rhs) const
        {
            if (mCallback && rhs.mCallback)
                return (*mCallback) < (*(rhs.mCallback));
            else
                return mCallback < rhs.mCallback;
        }

        /// Returns true if the callback has been set, or false if the callback is not set and is invalid.
        bool IsSet() const { return mCallback; }

        /// Invokes the callback.
        R operator()(T0 t0, T1 t1, T2 t2) const
        {
            if (mCallback)
                return (*mCallback)(t0, t1, t2);
            return R();
        }

        /// Invokes the callback. This function can sometimes be more convenient than the operator(), which does the same thing.
        R Call(T0 t0, T1 t1, T2 t2) const
        {
            if (mCallback)
                return (*mCallback)(t0, t1, t2);
            return R();
        }

    private:
        class Base
        {
        public:
            Base() {}
            virtual R           operator()(T0 t0, T1 t1, T2 t2)   = 0;
            virtual bool        operator==(const Base& rhs) const = 0;
            virtual bool        operator<(const Base& rhs) const  = 0;
            virtual void const* FreeFunction() const              = 0;
            virtual void const* MethodFunction() const            = 0;
            virtual void*       Comp() const                      = 0; // Returns a pointer used in comparisons.
        };

        class ChildFree : public Base
        {
        public:
            ChildFree(R (*function)(T0 t0, T1 t1, T2 t2))
                : mFunc(function)
            {
            }

            virtual R operator()(T0 t0, T1 t1, T2 t2) { return mFunc(t0, t1, t2); }

            virtual bool operator==(const Base& rhs) const
            {
                const ChildFree* const r = (const ChildFree*)rhs.FreeFunction();
                if (r)
                    return (mFunc == r->mFunc);
                else
                    return false;
            }

            virtual bool operator<(const Base& rhs) const
            {
                const ChildFree* const r = (const ChildFree*)rhs.FreeFunction();
                if (r)
                    return mFunc < r->mFunc;
                else
                    return true; // Free functions will always be less than methods (because comp returns 0).
            }

            virtual void const* FreeFunction() const { return this; }
            virtual void const* MethodFunction() const { return nullptr; }

            virtual void* Comp() const { return 0; }

        private:
            R (*const mFunc)(T0 t0, T1 t1, T2 t2);
        };

        template <typename C> class ChildMethod : public Base
        {
        public:
            ChildMethod(C* object, R (C::*function)(T0 t0, T1 t1, T2 t2))
                : mObj(object)
                , mFunc(function)
            {
            }

            virtual R operator()(T0 t0, T1 t1, T2 t2) { return (mObj->*mFunc)(t0, t1, t2); }

            virtual bool operator==(const Base& rhs) const
            {
                const ChildMethod<C>* const r = (const ChildMethod<C>*)rhs.MethodFunction();
                if (r)
                    return (mObj == r->mObj) && (mFunc == r->mFunc);
                else
                    return false;
            }

            virtual bool operator<(const Base& rhs) const
            {
                const ChildMethod<C>* const r = (const ChildMethod<C>*)rhs.MethodFunction();
                if (r)
                {
                    if (mObj != r->mObj)
                        return mObj < r->mObj;
                    else
                        return 0 > __private__::compare_mem((void const*)&mFunc, (void*)&(r->mFunc), (u32)sizeof(mFunc));
                }
                else
                    return mObj < rhs.Comp();
            }
            virtual void const* FreeFunction() const { return nullptr; }
            virtual void const* MethodFunction() const { return this; }

            virtual void* Comp() const { return mObj; }

        private:
            C* const mObj;
            R (C::*const mFunc)(T0 t0, T1 t1, T2 t2);
        };

        /// This class is only to find the worst case method pointer size.
        class unknown;

        char  mMem[sizeof(ChildMethod<unknown>)]; // Reserve memory for creating useful objects later.
        Base* mCallback;
    };

    /// Helper function to construct a callback without bothering to specify template parameters.
    template <typename C, typename R, typename T0, typename T1, typename T2> Callback3<R, T0, T1, T2> Make3(C* object, R (C::*function)(T0 t0, T1 t1, T2 t2)) { return Callback3<R, T0, T1, T2>(object, function); }

    /// Helper function to construct a callback without bothering to specify template parameters.
    template <typename R, typename T0, typename T1, typename T2> Callback3<R, T0, T1, T2> Make3(R (*function)(T0 t0, T1 t1, T2 t2)) { return Callback3<R, T0, T1, T2>(function); }

    /// Stores a callback for a function taking 4 parameters.
    ///\tparam R Callback function return type.
    template <typename R, typename T0, typename T1, typename T2, typename T3> class Callback4
    {
    public:
        /// Constructs the callback to a specific object and member function.
        ///\param object Pointer to the object to call upon. Care should be taken that this object remains valid as long as the callback may be invoked.
        ///\param function Member function address to call.
        template <typename C>
        Callback4(C* object, R (C::*function)(T0 t0, T1 t1, T2 t2, T3 t3))
            : mCallback(new(new_signature(), &mMem) ChildMethod<C>(object, function))
        {
        }

        /// Constructs the callback to a free function or static member function.
        ///\param function Free function address to call.
        Callback4(R (*function)(T0 t0, T1 t1, T2 t2, T3 t3))
            : mCallback(new(new_signature(), &mMem) ChildFree(function))
        {
        }

        /// Constructs a callback that can later be set.
        Callback4()
            : mCallback(0)
        {
        }

        Callback4(const Callback4& c)
            : mCallback(c.mCallback)
        {
            if (mCallback)
            {
                __private__::copy_mem(mMem, c.mMem, (u32)sizeof(mMem));
                mCallback = reinterpret_cast<Base*>(&mMem);
            }
        }

        Callback4& operator=(const Callback4& rhs)
        {
            mCallback = rhs.mCallback;
            if (mCallback)
            {
                __private__::copy_mem(mMem, rhs.mMem, (u32)sizeof(mMem));
                mCallback = reinterpret_cast<Base*>(&mMem);
            }

            return *this;
        }

        ~Callback4() {}

        /// Sets the callback to a specific object and member function.
        ///\param object Pointer to the object to call upon. Care should be taken that this object remains valid as long as the callback may be invoked.
        ///\param function Member function address to call.
        template <typename C> void Reset(C* object, R (C::*function)(T0 t0, T1 t1, T2 t2, T3 t3)) { mCallback = new (&mMem) ChildMethod<C>(object, function); }

        /// Sets the callback to a free function or static member function.
        ///\param function Free function address to call.
        void Reset(R (*function)(T0 t0, T1 t1, T2 t2, T3 t3)) { mCallback = new (&mMem) ChildFree(function); }

        /// Resests to callback to nothing.
        void Reset() { mCallback = 0; }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator==(const Callback4& rhs) const
        {
            if (mCallback && rhs.mCallback)
                return (*mCallback) == (*(rhs.mCallback));
            else
                return mCallback == rhs.mCallback;
        }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator!=(const Callback4& rhs) const { return !(*this == rhs); }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator<(const Callback4 rhs) const
        {
            if (mCallback && rhs.mCallback)
                return (*mCallback) < (*(rhs.mCallback));
            else
                return mCallback < rhs.mCallback;
        }

        /// Returns true if the callback has been set, or false if the callback is not set and is invalid.
        bool IsSet() const { return mCallback; }

        /// Invokes the callback.
        R operator()(T0 t0, T1 t1, T2 t2, T3 t3) const
        {
            if (mCallback)
                return (*mCallback)(t0, t1, t2, t3);
            return R();
        }

        /// Invokes the callback. This function can sometimes be more convenient than the operator(), which does the same thing.
        R Call(T0 t0, T1 t1, T2 t2, T3 t3) const
        {
            if (mCallback)
                return (*mCallback)(t0, t1, t2, t3);
            return R();
        }

    private:
        class Base
        {
        public:
            Base() {}
            virtual R           operator()(T0 t0, T1 t1, T2 t2, T3 t3) = 0;
            virtual bool        operator==(const Base& rhs) const      = 0;
            virtual bool        operator<(const Base& rhs) const       = 0;
            virtual void const* FreeFunction() const                   = 0;
            virtual void const* MethodFunction() const                 = 0;
            virtual void*       Comp() const                           = 0; // Returns a pointer used in comparisons.
        };

        class ChildFree : public Base
        {
        public:
            ChildFree(R (*function)(T0 t0, T1 t1, T2 t2, T3 t3))
                : mFunc(function)
            {
            }

            virtual R operator()(T0 t0, T1 t1, T2 t2, T3 t3) { return mFunc(t0, t1, t2, t3); }

            virtual bool operator==(const Base& rhs) const
            {
                const ChildFree* const r = (const ChildFree*)rhs.FreeFunction();
                if (r)
                    return (mFunc == r->mFunc);
                else
                    return false;
            }

            virtual bool operator<(const Base& rhs) const
            {
                const ChildFree* const r = (const ChildFree*)rhs.FreeFunction();
                if (r)
                    return mFunc < r->mFunc;
                else
                    return true; // Free functions will always be less than methods (because comp returns 0).
            }

            virtual void const* FreeFunction() const { return this; }
            virtual void const* MethodFunction() const { return nullptr; }

            virtual void* Comp() const { return 0; }

        private:
            R (*const mFunc)(T0 t0, T1 t1, T2 t2, T3 t3);
        };

        template <typename C> class ChildMethod : public Base
        {
        public:
            ChildMethod(C* object, R (C::*function)(T0 t0, T1 t1, T2 t2, T3 t3))
                : mObj(object)
                , mFunc(function)
            {
            }

            virtual R operator()(T0 t0, T1 t1, T2 t2, T3 t3) { return (mObj->*mFunc)(t0, t1, t2, t3); }

            virtual bool operator==(const Base& rhs) const
            {
                const ChildMethod<C>* const r = (const ChildMethod<C>*)rhs.MethodFunction();
                if (r)
                    return (mObj == r->mObj) && (mFunc == r->mFunc);
                else
                    return false;
            }

            virtual bool operator<(const Base& rhs) const
            {
                const ChildMethod<C>* const r = (const ChildMethod<C>*)rhs.MethodFunction();
                if (r)
                {
                    if (mObj != r->mObj)
                        return mObj < r->mObj;
                    else
                        return 0 > __private__::compare_mem((void const*)&mFunc, (void*)&(r->mFunc), (u32)sizeof(mFunc));
                }
                else
                    return mObj < rhs.Comp();
            }
            virtual void const* FreeFunction() const { return nullptr; }
            virtual void const* MethodFunction() const { return this; }

            virtual void* Comp() const { return mObj; }

        private:
            C* const mObj;
            R (C::*const mFunc)(T0 t0, T1 t1, T2 t2, T3 t3);
        };

        /// This class is only to find the worst case method pointer size.
        class unknown;

        char  mMem[sizeof(ChildMethod<unknown>)]; // Reserve memory for creating useful objects later.
        Base* mCallback;
    };

    /// Helper function to construct a callback without bothering to specify template parameters.
    template <typename C, typename R, typename T0, typename T1, typename T2, typename T3> Callback4<R, T0, T1, T2, T3> Make4(C* object, R (C::*function)(T0 t0, T1 t1, T2 t2, T3 t3)) { return Callback4<R, T0, T1, T2, T3>(object, function); }

    /// Helper function to construct a callback without bothering to specify template parameters.
    template <typename R, typename T0, typename T1, typename T2, typename T3> Callback4<R, T0, T1, T2, T3> Make4(R (*function)(T0 t0, T1 t1, T2 t2, T3 t3)) { return Callback4<R, T0, T1, T2, T3>(function); }

    /// Stores a callback for a function taking 5 parameters.
    ///\tparam R Callback function return type.
    template <typename R, typename T0, typename T1, typename T2, typename T3, typename T4> class Callback5
    {
    public:
        /// Constructs the callback to a specific object and member function.
        ///\param object Pointer to the object to call upon. Care should be taken that this object remains valid as long as the callback may be invoked.
        ///\param function Member function address to call.
        template <typename C>
        Callback5(C* object, R (C::*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4))
            : mCallback(new(new_signature(), &mMem) ChildMethod<C>(object, function))
        {
        }

        /// Constructs the callback to a free function or static member function.
        ///\param function Free function address to call.
        Callback5(R (*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4))
            : mCallback(new(new_signature(), &mMem) ChildFree(function))
        {
        }

        /// Constructs a callback that can later be set.
        Callback5()
            : mCallback(0)
        {
        }

        Callback5(const Callback5& c)
            : mCallback(c.mCallback)
        {
            if (mCallback)
            {
                __private__::copy_mem(mMem, c.mMem, (u32)sizeof(mMem));
                mCallback = reinterpret_cast<Base*>(&mMem);
            }
        }

        Callback5& operator=(const Callback5& rhs)
        {
            mCallback = rhs.mCallback;
            if (mCallback)
            {
                __private__::copy_mem(mMem, rhs.mMem, (u32)sizeof(mMem));
                mCallback = reinterpret_cast<Base*>(&mMem);
            }

            return *this;
        }

        ~Callback5() {}

        /// Sets the callback to a specific object and member function.
        ///\param object Pointer to the object to call upon. Care should be taken that this object remains valid as long as the callback may be invoked.
        ///\param function Member function address to call.
        template <typename C> void Reset(C* object, R (C::*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4)) { mCallback = new (&mMem) ChildMethod<C>(object, function); }

        /// Sets the callback to a free function or static member function.
        ///\param function Free function address to call.
        void Reset(R (*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4)) { mCallback = new (&mMem) ChildFree(function); }

        /// Resests to callback to nothing.
        void Reset() { mCallback = 0; }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator==(const Callback5& rhs) const
        {
            if (mCallback && rhs.mCallback)
                return (*mCallback) == (*(rhs.mCallback));
            else
                return mCallback == rhs.mCallback;
        }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator!=(const Callback5& rhs) const { return !(*this == rhs); }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator<(const Callback5 rhs) const
        {
            if (mCallback && rhs.mCallback)
                return (*mCallback) < (*(rhs.mCallback));
            else
                return mCallback < rhs.mCallback;
        }

        /// Returns true if the callback has been set, or false if the callback is not set and is invalid.
        bool IsSet() const { return mCallback; }

        /// Invokes the callback.
        R operator()(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4) const
        {
            if (mCallback)
                return (*mCallback)(t0, t1, t2, t3, t4);
            return R();
        }

        /// Invokes the callback. This function can sometimes be more convenient than the operator(), which does the same thing.
        R Call(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4) const
        {
            if (mCallback)
                return (*mCallback)(t0, t1, t2, t3, t4);
            return R();
        }

    private:
        class Base
        {
        public:
            Base() {}
            virtual R           operator()(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4) = 0;
            virtual bool        operator==(const Base& rhs) const             = 0;
            virtual bool        operator<(const Base& rhs) const              = 0;
            virtual void const* FreeFunction() const                          = 0;
            virtual void const* MethodFunction() const                        = 0;
            virtual void*       Comp() const                                  = 0; // Returns a pointer used in comparisons.
        };

        class ChildFree : public Base
        {
        public:
            ChildFree(R (*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4))
                : mFunc(function)
            {
            }

            virtual R operator()(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4) { return mFunc(t0, t1, t2, t3, t4); }

            virtual bool operator==(const Base& rhs) const
            {
                const ChildFree* const r = (const ChildFree*)rhs.FreeFunction();
                if (r)
                    return (mFunc == r->mFunc);
                else
                    return false;
            }

            virtual bool operator<(const Base& rhs) const
            {
                const ChildFree* const r = (const ChildFree*)rhs.FreeFunction();
                if (r)
                    return mFunc < r->mFunc;
                else
                    return true; // Free functions will always be less than methods (because comp returns 0).
            }

            virtual void const* FreeFunction() const { return this; }
            virtual void const* MethodFunction() const { return nullptr; }

            virtual void* Comp() const { return 0; }

        private:
            R (*const mFunc)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4);
        };

        template <typename C> class ChildMethod : public Base
        {
        public:
            ChildMethod(C* object, R (C::*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4))
                : mObj(object)
                , mFunc(function)
            {
            }

            virtual R operator()(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4) { return (mObj->*mFunc)(t0, t1, t2, t3, t4); }

            virtual bool operator==(const Base& rhs) const
            {
                const ChildMethod<C>* const r = (const ChildMethod<C>*)rhs.MethodFunction();
                if (r)
                    return (mObj == r->mObj) && (mFunc == r->mFunc);
                else
                    return false;
            }

            virtual bool operator<(const Base& rhs) const
            {
                const ChildMethod<C>* const r = (const ChildMethod<C>*)rhs.MethodFunction();
                if (r)
                {
                    if (mObj != r->mObj)
                        return mObj < r->mObj;
                    else
                        return 0 > __private__::compare_mem((void const*)&mFunc, (void*)&(r->mFunc), (u32)sizeof(mFunc));
                }
                else
                    return mObj < rhs.Comp();
            }
            virtual void const* FreeFunction() const { return nullptr; }
            virtual void const* MethodFunction() const { return this; }

            virtual void* Comp() const { return mObj; }

        private:
            C* const mObj;
            R (C::*const mFunc)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4);
        };

        /// This class is only to find the worst case method pointer size.
        class unknown;

        char  mMem[sizeof(ChildMethod<unknown>)]; // Reserve memory for creating useful objects later.
        Base* mCallback;
    };

    /// Helper function to construct a callback without bothering to specify template parameters.
    template <typename C, typename R, typename T0, typename T1, typename T2, typename T3, typename T4> Callback5<R, T0, T1, T2, T3, T4> Make5(C* object, R (C::*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4)) { return Callback5<R, T0, T1, T2, T3, T4>(object, function); }

    /// Helper function to construct a callback without bothering to specify template parameters.
    template <typename R, typename T0, typename T1, typename T2, typename T3, typename T4> Callback5<R, T0, T1, T2, T3, T4> Make5(R (*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4)) { return Callback5<R, T0, T1, T2, T3, T4>(function); }

    /// Stores a callback for a function taking 6 parameters.
    ///\tparam R Callback function return type.
    template <typename R, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5> class Callback6
    {
    public:
        /// Constructs the callback to a specific object and member function.
        ///\param object Pointer to the object to call upon. Care should be taken that this object remains valid as long as the callback may be invoked.
        ///\param function Member function address to call.
        template <typename C>
        Callback6(C* object, R (C::*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5))
            : mCallback(new(new_signature(), &mMem) ChildMethod<C>(object, function))
        {
        }

        /// Constructs the callback to a free function or static member function.
        ///\param function Free function address to call.
        Callback6(R (*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5))
            : mCallback(new(new_signature(), &mMem) ChildFree(function))
        {
        }

        /// Constructs a callback that can later be set.
        Callback6()
            : mCallback(0)
        {
        }

        Callback6(const Callback6& c)
            : mCallback(c.mCallback)
        {
            if (mCallback)
            {
                __private__::copy_mem(mMem, c.mMem, (u32)sizeof(mMem));
                mCallback = reinterpret_cast<Base*>(&mMem);
            }
        }

        Callback6& operator=(const Callback6& rhs)
        {
            mCallback = rhs.mCallback;
            if (mCallback)
            {
                __private__::copy_mem(mMem, rhs.mMem, (u32)sizeof(mMem));
                mCallback = reinterpret_cast<Base*>(&mMem);
            }

            return *this;
        }

        ~Callback6() {}

        /// Sets the callback to a specific object and member function.
        ///\param object Pointer to the object to call upon. Care should be taken that this object remains valid as long as the callback may be invoked.
        ///\param function Member function address to call.
        template <typename C> void Reset(C* object, R (C::*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)) { mCallback = new (&mMem) ChildMethod<C>(object, function); }

        /// Sets the callback to a free function or static member function.
        ///\param function Free function address to call.
        void Reset(R (*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)) { mCallback = new (&mMem) ChildFree(function); }

        /// Resests to callback to nothing.
        void Reset() { mCallback = 0; }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator==(const Callback6& rhs) const
        {
            if (mCallback && rhs.mCallback)
                return (*mCallback) == (*(rhs.mCallback));
            else
                return mCallback == rhs.mCallback;
        }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator!=(const Callback6& rhs) const { return !(*this == rhs); }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator<(const Callback6 rhs) const
        {
            if (mCallback && rhs.mCallback)
                return (*mCallback) < (*(rhs.mCallback));
            else
                return mCallback < rhs.mCallback;
        }

        /// Returns true if the callback has been set, or false if the callback is not set and is invalid.
        bool IsSet() const { return mCallback; }

        /// Invokes the callback.
        R operator()(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5) const
        {
            if (mCallback)
                return (*mCallback)(t0, t1, t2, t3, t4, t5);
            return R();
        }

        /// Invokes the callback. This function can sometimes be more convenient than the operator(), which does the same thing.
        R Call(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5) const
        {
            if (mCallback)
                return (*mCallback)(t0, t1, t2, t3, t4, t5);
            return R();
        }

    private:
        class Base
        {
        public:
            Base() {}
            virtual R           operator()(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5) = 0;
            virtual bool        operator==(const Base& rhs) const                    = 0;
            virtual bool        operator<(const Base& rhs) const                     = 0;
            virtual void const* FreeFunction() const                                 = 0;
            virtual void const* MethodFunction() const                               = 0;
            virtual void*       Comp() const                                         = 0; // Returns a pointer used in comparisons.
        };

        class ChildFree : public Base
        {
        public:
            ChildFree(R (*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5))
                : mFunc(function)
            {
            }

            virtual R operator()(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5) { return mFunc(t0, t1, t2, t3, t4, t5); }

            virtual bool operator==(const Base& rhs) const
            {
                const ChildFree* const r = (const ChildFree*)rhs.FreeFunction();
                if (r)
                    return (mFunc == r->mFunc);
                else
                    return false;
            }

            virtual bool operator<(const Base& rhs) const
            {
                const ChildFree* const r = (const ChildFree*)rhs.FreeFunction();
                if (r)
                    return mFunc < r->mFunc;
                else
                    return true; // Free functions will always be less than methods (because comp returns 0).
            }

            virtual void const* FreeFunction() const { return this; }
            virtual void const* MethodFunction() const { return nullptr; }

            virtual void* Comp() const { return 0; }

        private:
            R (*const mFunc)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5);
        };

        template <typename C> class ChildMethod : public Base
        {
        public:
            ChildMethod(C* object, R (C::*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5))
                : mObj(object)
                , mFunc(function)
            {
            }

            virtual R operator()(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5) { return (mObj->*mFunc)(t0, t1, t2, t3, t4, t5); }

            virtual bool operator==(const Base& rhs) const
            {
                const ChildMethod<C>* const r = (const ChildMethod<C>*)rhs.MethodFunction();
                if (r)
                    return (mObj == r->mObj) && (mFunc == r->mFunc);
                else
                    return false;
            }

            virtual bool operator<(const Base& rhs) const
            {
                const ChildMethod<C>* const r = (const ChildMethod<C>*)rhs.MethodFunction();
                if (r)
                {
                    if (mObj != r->mObj)
                        return mObj < r->mObj;
                    else
                        return 0 > __private__::compare_mem((void const*)&mFunc, (void*)&(r->mFunc), (u32)sizeof(mFunc));
                }
                else
                    return mObj < rhs.Comp();
            }
            virtual void const* FreeFunction() const { return nullptr; }
            virtual void const* MethodFunction() const { return this; }

            virtual void* Comp() const { return mObj; }

        private:
            C* const mObj;
            R (C::*const mFunc)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5);
        };

        /// This class is only to find the worst case method pointer size.
        class unknown;

        char  mMem[sizeof(ChildMethod<unknown>)]; // Reserve memory for creating useful objects later.
        Base* mCallback;
    };

    /// Helper function to construct a callback without bothering to specify template parameters.
    template <typename C, typename R, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5> Callback6<R, T0, T1, T2, T3, T4, T5> Make6(C* object, R (C::*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)) { return Callback6<R, T0, T1, T2, T3, T4, T5>(object, function); }

    /// Helper function to construct a callback without bothering to specify template parameters.
    template <typename R, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5> Callback6<R, T0, T1, T2, T3, T4, T5> Make6(R (*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)) { return Callback6<R, T0, T1, T2, T3, T4, T5>(function); }

    /// Stores a callback for a function taking 7 parameters.
    ///\tparam R Callback function return type.
    template <typename R, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> class Callback7
    {
    public:
        /// Constructs the callback to a specific object and member function.
        ///\param object Pointer to the object to call upon. Care should be taken that this object remains valid as long as the callback may be invoked.
        ///\param function Member function address to call.
        template <typename C>
        Callback7(C* object, R (C::*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6))
            : mCallback(new(new_signature(), &mMem) ChildMethod<C>(object, function))
        {
        }

        /// Constructs the callback to a free function or static member function.
        ///\param function Free function address to call.
        Callback7(R (*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6))
            : mCallback(new(new_signature(), &mMem) ChildFree(function))
        {
        }

        /// Constructs a callback that can later be set.
        Callback7()
            : mCallback(0)
        {
        }

        Callback7(const Callback7& c)
            : mCallback(c.mCallback)
        {
            if (mCallback)
            {
                __private__::copy_mem(mMem, c.mMem, (u32)sizeof(mMem));
                mCallback = reinterpret_cast<Base*>(&mMem);
            }
        }

        Callback7& operator=(const Callback7& rhs)
        {
            mCallback = rhs.mCallback;
            if (mCallback)
            {
                __private__::copy_mem(mMem, rhs.mMem, (u32)sizeof(mMem));
                mCallback = reinterpret_cast<Base*>(&mMem);
            }

            return *this;
        }

        ~Callback7() {}

        /// Sets the callback to a specific object and member function.
        ///\param object Pointer to the object to call upon. Care should be taken that this object remains valid as long as the callback may be invoked.
        ///\param function Member function address to call.
        template <typename C> void Reset(C* object, R (C::*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)) { mCallback = new (&mMem) ChildMethod<C>(object, function); }

        /// Sets the callback to a free function or static member function.
        ///\param function Free function address to call.
        void Reset(R (*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)) { mCallback = new (&mMem) ChildFree(function); }

        /// Resests to callback to nothing.
        void Reset() { mCallback = 0; }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator==(const Callback7& rhs) const
        {
            if (mCallback && rhs.mCallback)
                return (*mCallback) == (*(rhs.mCallback));
            else
                return mCallback == rhs.mCallback;
        }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator!=(const Callback7& rhs) const { return !(*this == rhs); }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator<(const Callback7 rhs) const
        {
            if (mCallback && rhs.mCallback)
                return (*mCallback) < (*(rhs.mCallback));
            else
                return mCallback < rhs.mCallback;
        }

        /// Returns true if the callback has been set, or false if the callback is not set and is invalid.
        bool IsSet() const { return mCallback; }

        /// Invokes the callback.
        R operator()(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6) const
        {
            if (mCallback)
                return (*mCallback)(t0, t1, t2, t3, t4, t5, t6);
            return R();
        }

        /// Invokes the callback. This function can sometimes be more convenient than the operator(), which does the same thing.
        R Call(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6) const
        {
            if (mCallback)
                return (*mCallback)(t0, t1, t2, t3, t4, t5, t6);
            return R();
        }

    private:
        class Base
        {
        public:
            Base() {}
            virtual R           operator()(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6) = 0;
            virtual bool        operator==(const Base& rhs) const                           = 0;
            virtual bool        operator<(const Base& rhs) const                            = 0;
            virtual void const* FreeFunction() const                                        = 0;
            virtual void const* MethodFunction() const                                      = 0;
            virtual void*       Comp() const                                                = 0; // Returns a pointer used in comparisons.
        };

        class ChildFree : public Base
        {
        public:
            ChildFree(R (*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6))
                : mFunc(function)
            {
            }

            virtual R operator()(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6) { return mFunc(t0, t1, t2, t3, t4, t5, t6); }

            virtual bool operator==(const Base& rhs) const
            {
                const ChildFree* const r = (const ChildFree*)rhs.FreeFunction();
                if (r)
                    return (mFunc == r->mFunc);
                else
                    return false;
            }

            virtual bool operator<(const Base& rhs) const
            {
                const ChildFree* const r = (const ChildFree*)rhs.FreeFunction();
                if (r)
                    return mFunc < r->mFunc;
                else
                    return true; // Free functions will always be less than methods (because comp returns 0).
            }

            virtual void const* FreeFunction() const { return this; }
            virtual void const* MethodFunction() const { return nullptr; }

            virtual void* Comp() const { return 0; }

        private:
            R (*const mFunc)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6);
        };

        template <typename C> class ChildMethod : public Base
        {
        public:
            ChildMethod(C* object, R (C::*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6))
                : mObj(object)
                , mFunc(function)
            {
            }

            virtual R operator()(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6) { return (mObj->*mFunc)(t0, t1, t2, t3, t4, t5, t6); }

            virtual bool operator==(const Base& rhs) const
            {
                const ChildMethod<C>* const r = (const ChildMethod<C>*)rhs.MethodFunction();
                if (r)
                    return (mObj == r->mObj) && (mFunc == r->mFunc);
                else
                    return false;
            }

            virtual bool operator<(const Base& rhs) const
            {
                const ChildMethod<C>* const r = (const ChildMethod<C>*)rhs.MethodFunction();
                if (r)
                {
                    if (mObj != r->mObj)
                        return mObj < r->mObj;
                    else
                        return 0 > __private__::compare_mem((void const*)&mFunc, (void*)&(r->mFunc), (u32)sizeof(mFunc));
                }
                else
                    return mObj < rhs.Comp();
            }
            virtual void const* FreeFunction() const { return nullptr; }
            virtual void const* MethodFunction() const { return this; }

            virtual void* Comp() const { return mObj; }

        private:
            C* const mObj;
            R (C::*const mFunc)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6);
        };

        /// This class is only to find the worst case method pointer size.
        class unknown;

        char  mMem[sizeof(ChildMethod<unknown>)]; // Reserve memory for creating useful objects later.
        Base* mCallback;
    };

    /// Helper function to construct a callback without bothering to specify template parameters.
    template <typename C, typename R, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> Callback7<R, T0, T1, T2, T3, T4, T5, T6> Make7(C* object, R (C::*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)) { return Callback7<R, T0, T1, T2, T3, T4, T5, T6>(object, function); }

    /// Helper function to construct a callback without bothering to specify template parameters.
    template <typename R, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> Callback7<R, T0, T1, T2, T3, T4, T5, T6> Make7(R (*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)) { return Callback7<R, T0, T1, T2, T3, T4, T5, T6>(function); }

    /// Stores a callback for a function taking 8 parameters.
    ///\tparam R Callback function return type.
    template <typename R, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> class Callback8
    {
    public:
        /// Constructs the callback to a specific object and member function.
        ///\param object Pointer to the object to call upon. Care should be taken that this object remains valid as long as the callback may be invoked.
        ///\param function Member function address to call.
        template <typename C>
        Callback8(C* object, R (C::*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7))
            : mCallback(new(new_signature(), &mMem) ChildMethod<C>(object, function))
        {
        }

        /// Constructs the callback to a free function or static member function.
        ///\param function Free function address to call.
        Callback8(R (*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7))
            : mCallback(new(new_signature(), &mMem) ChildFree(function))
        {
        }

        /// Constructs a callback that can later be set.
        Callback8()
            : mCallback(0)
        {
        }

        Callback8(const Callback8& c)
            : mCallback(c.mCallback)
        {
            if (mCallback)
            {
                __private__::copy_mem(mMem, c.mMem, (u32)sizeof(mMem));
                mCallback = reinterpret_cast<Base*>(&mMem);
            }
        }

        Callback8& operator=(const Callback8& rhs)
        {
            mCallback = rhs.mCallback;
            if (mCallback)
            {
                __private__::copy_mem(mMem, rhs.mMem, (u32)sizeof(mMem));
                mCallback = reinterpret_cast<Base*>(&mMem);
            }

            return *this;
        }

        ~Callback8() {}

        /// Sets the callback to a specific object and member function.
        ///\param object Pointer to the object to call upon. Care should be taken that this object remains valid as long as the callback may be invoked.
        ///\param function Member function address to call.
        template <typename C> void Reset(C* object, R (C::*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7)) { mCallback = new (&mMem) ChildMethod<C>(object, function); }

        /// Sets the callback to a free function or static member function.
        ///\param function Free function address to call.
        void Reset(R (*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7)) { mCallback = new (&mMem) ChildFree(function); }

        /// Resests to callback to nothing.
        void Reset() { mCallback = 0; }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator==(const Callback8& rhs) const
        {
            if (mCallback && rhs.mCallback)
                return (*mCallback) == (*(rhs.mCallback));
            else
                return mCallback == rhs.mCallback;
        }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator!=(const Callback8& rhs) const { return !(*this == rhs); }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator<(const Callback8 rhs) const
        {
            if (mCallback && rhs.mCallback)
                return (*mCallback) < (*(rhs.mCallback));
            else
                return mCallback < rhs.mCallback;
        }

        /// Returns true if the callback has been set, or false if the callback is not set and is invalid.
        bool IsSet() const { return mCallback; }

        /// Invokes the callback.
        R operator()(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7) const
        {
            if (mCallback)
                return (*mCallback)(t0, t1, t2, t3, t4, t5, t6, t7);
            return R();
        }

        /// Invokes the callback. This function can sometimes be more convenient than the operator(), which does the same thing.
        R Call(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7) const
        {
            if (mCallback)
                return (*mCallback)(t0, t1, t2, t3, t4, t5, t6, t7);
            return R();
        }

    private:
        class Base
        {
        public:
            Base() {}
            virtual R           operator()(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7) = 0;
            virtual bool        operator==(const Base& rhs) const                                  = 0;
            virtual bool        operator<(const Base& rhs) const                                   = 0;
            virtual void const* FreeFunction() const                                               = 0;
            virtual void const* MethodFunction() const                                             = 0;
            virtual void*       Comp() const                                                       = 0; // Returns a pointer used in comparisons.
        };

        class ChildFree : public Base
        {
        public:
            ChildFree(R (*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7))
                : mFunc(function)
            {
            }

            virtual R operator()(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7) { return mFunc(t0, t1, t2, t3, t4, t5, t6, t7); }

            virtual bool operator==(const Base& rhs) const
            {
                const ChildFree* const r = (const ChildFree*)rhs.FreeFunction();
                if (r)
                    return (mFunc == r->mFunc);
                else
                    return false;
            }

            virtual bool operator<(const Base& rhs) const
            {
                const ChildFree* const r = (const ChildFree*)rhs.FreeFunction();
                if (r)
                    return mFunc < r->mFunc;
                else
                    return true; // Free functions will always be less than methods (because comp returns 0).
            }

            virtual void const* FreeFunction() const { return this; }
            virtual void const* MethodFunction() const { return nullptr; }

            virtual void* Comp() const { return 0; }

        private:
            R (*const mFunc)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7);
        };

        template <typename C> class ChildMethod : public Base
        {
        public:
            ChildMethod(C* object, R (C::*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7))
                : mObj(object)
                , mFunc(function)
            {
            }

            virtual R operator()(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7) { return (mObj->*mFunc)(t0, t1, t2, t3, t4, t5, t6, t7); }

            virtual bool operator==(const Base& rhs) const
            {
                const ChildMethod<C>* const r = (const ChildMethod<C>*)rhs.MethodFunction();
                if (r)
                    return (mObj == r->mObj) && (mFunc == r->mFunc);
                else
                    return false;
            }

            virtual bool operator<(const Base& rhs) const
            {
                const ChildMethod<C>* const r = (const ChildMethod<C>*)rhs.MethodFunction();
                if (r)
                {
                    if (mObj != r->mObj)
                        return mObj < r->mObj;
                    else
                        return 0 > __private__::compare_mem((void const*)&mFunc, (void*)&(r->mFunc), (u32)sizeof(mFunc));
                }
                else
                    return mObj < rhs.Comp();
            }
            virtual void const* FreeFunction() const { return nullptr; }
            virtual void const* MethodFunction() const { return this; }

            virtual void* Comp() const { return mObj; }

        private:
            C* const mObj;
            R (C::*const mFunc)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7);
        };

        /// This class is only to find the worst case method pointer size.
        class unknown;

        char  mMem[sizeof(ChildMethod<unknown>)]; // Reserve memory for creating useful objects later.
        Base* mCallback;
    };

    /// Helper function to construct a callback without bothering to specify template parameters.
    template <typename C, typename R, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> Callback8<R, T0, T1, T2, T3, T4, T5, T6, T7> Make8(C* object, R (C::*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7)) { return Callback8<R, T0, T1, T2, T3, T4, T5, T6, T7>(object, function); }

    /// Helper function to construct a callback without bothering to specify template parameters.
    template <typename R, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> Callback8<R, T0, T1, T2, T3, T4, T5, T6, T7> Make8(R (*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7)) { return Callback8<R, T0, T1, T2, T3, T4, T5, T6, T7>(function); }

    /// Stores a callback for a function taking 9 parameters.
    ///\tparam R Callback function return type.
    template <typename R, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8> class Callback9
    {
    public:
        /// Constructs the callback to a specific object and member function.
        ///\param object Pointer to the object to call upon. Care should be taken that this object remains valid as long as the callback may be invoked.
        ///\param function Member function address to call.
        template <typename C>
        Callback9(C* object, R (C::*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8))
            : mCallback(new(new_signature(), &mMem) ChildMethod<C>(object, function))
        {
        }

        /// Constructs the callback to a free function or static member function.
        ///\param function Free function address to call.
        Callback9(R (*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8))
            : mCallback(new(new_signature(), &mMem) ChildFree(function))
        {
        }

        /// Constructs a callback that can later be set.
        Callback9()
            : mCallback(0)
        {
        }

        Callback9(const Callback9& c)
            : mCallback(c.mCallback)
        {
            if (mCallback)
            {
                __private__::copy_mem(mMem, c.mMem, (u32)sizeof(mMem));
                mCallback = reinterpret_cast<Base*>(&mMem);
            }
        }

        Callback9& operator=(const Callback9& rhs)
        {
            mCallback = rhs.mCallback;
            if (mCallback)
            {
                __private__::copy_mem(mMem, rhs.mMem, (u32)sizeof(mMem));
                mCallback = reinterpret_cast<Base*>(&mMem);
            }

            return *this;
        }

        ~Callback9() {}

        /// Sets the callback to a specific object and member function.
        ///\param object Pointer to the object to call upon. Care should be taken that this object remains valid as long as the callback may be invoked.
        ///\param function Member function address to call.
        template <typename C> void Reset(C* object, R (C::*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8)) { mCallback = new (&mMem) ChildMethod<C>(object, function); }

        /// Sets the callback to a free function or static member function.
        ///\param function Free function address to call.
        void Reset(R (*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8)) { mCallback = new (&mMem) ChildFree(function); }

        /// Resests to callback to nothing.
        void Reset() { mCallback = 0; }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator==(const Callback9& rhs) const
        {
            if (mCallback && rhs.mCallback)
                return (*mCallback) == (*(rhs.mCallback));
            else
                return mCallback == rhs.mCallback;
        }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator!=(const Callback9& rhs) const { return !(*this == rhs); }

        /// Note that comparison operators may not work with virtual function callbacks.
        bool operator<(const Callback9 rhs) const
        {
            if (mCallback && rhs.mCallback)
                return (*mCallback) < (*(rhs.mCallback));
            else
                return mCallback < rhs.mCallback;
        }

        /// Returns true if the callback has been set, or false if the callback is not set and is invalid.
        bool IsSet() const { return mCallback; }

        /// Invokes the callback.
        R operator()(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8) const
        {
            if (mCallback)
                return (*mCallback)(t0, t1, t2, t3, t4, t5, t6, t7, t8);
            return R();
        }

        /// Invokes the callback. This function can sometimes be more convenient than the operator(), which does the same thing.
        R Call(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8) const
        {
            if (mCallback)
                return (*mCallback)(t0, t1, t2, t3, t4, t5, t6, t7, t8);
            return R();
        }

    private:
        class Base
        {
        public:
            Base() {}
            virtual R           operator()(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8) = 0;
            virtual bool        operator==(const Base& rhs) const                                         = 0;
            virtual bool        operator<(const Base& rhs) const                                          = 0;
            virtual void const* FreeFunction() const                                                      = 0;
            virtual void const* MethodFunction() const                                                    = 0;
            virtual void*       Comp() const                                                              = 0; // Returns a pointer used in comparisons.
        };

        class ChildFree : public Base
        {
        public:
            ChildFree(R (*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8))
                : mFunc(function)
            {
            }

            virtual R operator()(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8) { return mFunc(t0, t1, t2, t3, t4, t5, t6, t7, t8); }

            virtual bool operator==(const Base& rhs) const
            {
                const ChildFree* const r = (const ChildFree*)rhs.FreeFunction();
                if (r)
                    return (mFunc == r->mFunc);
                else
                    return false;
            }

            virtual bool operator<(const Base& rhs) const
            {
                const ChildFree* const r = (const ChildFree*)rhs.FreeFunction();
                if (r)
                    return mFunc < r->mFunc;
                else
                    return true; // Free functions will always be less than methods (because comp returns 0).
            }

            virtual void const* FreeFunction() const { return this; }
            virtual void const* MethodFunction() const { return nullptr; }

            virtual void* Comp() const { return 0; }

        private:
            R (*const mFunc)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8);
        };

        template <typename C> class ChildMethod : public Base
        {
        public:
            ChildMethod(C* object, R (C::*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8))
                : mObj(object)
                , mFunc(function)
            {
            }

            virtual R operator()(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8) { return (mObj->*mFunc)(t0, t1, t2, t3, t4, t5, t6, t7, t8); }

            virtual bool operator==(const Base& rhs) const
            {
                const ChildMethod<C>* const r = (const ChildMethod<C>*)rhs.MethodFunction();
                if (r)
                    return (mObj == r->mObj) && (mFunc == r->mFunc);
                else
                    return false;
            }

            virtual bool operator<(const Base& rhs) const
            {
                const ChildMethod<C>* const r = (const ChildMethod<C>*)rhs.MethodFunction();
                if (r)
                {
                    if (mObj != r->mObj)
                        return mObj < r->mObj;
                    else
                        return 0 > __private__::compare_mem((void const*)&mFunc, (void*)&(r->mFunc), (u32)sizeof(mFunc));
                }
                else
                    return mObj < rhs.Comp();
            }
            virtual void const* FreeFunction() const { return nullptr; }
            virtual void const* MethodFunction() const { return this; }

            virtual void* Comp() const { return mObj; }

        private:
            C* const mObj;
            R (C::*const mFunc)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8);
        };

        /// This class is only to find the worst case method pointer size.
        class unknown;

        char  mMem[sizeof(ChildMethod<unknown>)]; // Reserve memory for creating useful objects later.
        Base* mCallback;
    };

    /// Helper function to construct a callback without bothering to specify template parameters.
    template <typename C, typename R, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8> Callback9<R, T0, T1, T2, T3, T4, T5, T6, T7, T8> Make9(C* object, R (C::*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8)) { return Callback9<R, T0, T1, T2, T3, T4, T5, T6, T7, T8>(object, function); }

    /// Helper function to construct a callback without bothering to specify template parameters.
    template <typename R, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8> Callback9<R, T0, T1, T2, T3, T4, T5, T6, T7, T8> Make9(R (*function)(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8)) { return Callback9<R, T0, T1, T2, T3, T4, T5, T6, T7, T8>(function); }

} // namespace ncore

#endif
