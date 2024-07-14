#include "ccore/c_target.h"
#include "cbase/c_context.h"
#include "cbase/c_hbb.h"
#include "cmsg/c_delegate.h"

#include "cunittest/cunittest.h"

using namespace ncore;

UNITTEST_SUITE_BEGIN(delegate)
{
    UNITTEST_FIXTURE(no_return)
    {
        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

        struct Dog
        {
            Dog()
                : m_volume(0)
            {
            }
            int  m_volume;
            void Bark(int volume) { m_volume = volume; }
        };

        struct Cat
        {
            Cat()
                : m_volume(0)
            {
            }
            int  m_volume;
            void Meow(int volume) { m_volume = volume; }
        };

        Dog spot, rover; // We have two dogs
        Cat felix;       // and one cat.

        // Define a normal function.
        int  volume = 0;
        void Func(int a) { volume = a; }

        UNITTEST_TEST(dogs_and_cat)
        {
            // Define a callback to a function returning void and taking
            // one int parameter.
            ncore::Callback1<void, int> speak;

            // Point this callback at spot's Bark method.
            speak.Reset(&spot, &Dog::Bark);
            speak(50); // Spot barks loudly.
            CHECK_EQUAL(50, spot.m_volume);

            speak.Reset(&rover, &Dog::Bark);
            speak(60); // Rovers lets out a mighty bark.
            CHECK_EQUAL(60, rover.m_volume);

            speak.Reset(&felix, &Cat::Meow);
            speak(30); // Felix meows.
            CHECK_EQUAL(30, felix.m_volume);
        }

        UNITTEST_TEST(free)
        {
            ncore::Callback1<void, int> delegate;

            // Callbacks can be set to free functions.
            delegate = Func;
            delegate(10);
            CHECK_EQUAL(10, volume);

            // Copy and assignment operators are well defined.
            ncore::Callback1<void, int> copy = delegate;
            ASSERT(copy == delegate);
            copy(20);
            CHECK_EQUAL(20, volume);

            // Callbacks can be set to null.
            copy.Reset();
            ASSERT(!copy.IsSet());
            copy(30);
            CHECK_EQUAL(20, volume);
        }
    }

    UNITTEST_FIXTURE(with_return)
    {
        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

        struct Dog
        {
            int m_volume{0};
            int Bark(int volume)
            {
                m_volume += volume;
                return m_volume;
            }
        };

        struct Cat
        {
            int m_volume{0};
            int Meow(int volume)
            {
                m_volume += volume;
                return m_volume;
            }
        };

        Dog spot, rover; // We have two dogs
        Cat felix;       // and one cat.

        // Define a normal function.
        int  s_volume = 0;
        int Func(int a) { s_volume += a; return s_volume; }

        UNITTEST_TEST(dogs_and_cat)
        {
            // Define a callback to a function returning void and taking
            // one int parameter.
            ncore::Callback1<int, int> speak;

            // Point this callback at spot's Bark method.
            speak.Reset(&spot, &Dog::Bark);
            int v1 = speak(50); // Spot barks loudly.
            CHECK_EQUAL(50, v1);

            speak.Reset(&rover, &Dog::Bark);
            int v2 = speak(60); // Rovers lets out a mighty bark.
            CHECK_EQUAL(60, v2);

            speak.Reset(&felix, &Cat::Meow);
            int v3 = speak(30); // Felix meows.
            CHECK_EQUAL(30, v3);
        }

        UNITTEST_TEST(free)
        {
            ncore::Callback1<int, int> delegate;

            // Callbacks can be set to free functions.
            delegate = Func;
            int v1 = delegate(10);
            CHECK_EQUAL(10, v1);

            // Copy and assignment operators are well defined.
            ncore::Callback1<int, int> copy = delegate;
            ASSERT(copy == delegate);
            int v2 = copy(20);
            CHECK_EQUAL(10 + 20, v2);

            // Callbacks can be set to null.
            copy.Reset();
            ASSERT(!copy.IsSet());
            int v3 = copy(30);
            CHECK_EQUAL(0, v3);
        }
    }
}
UNITTEST_SUITE_END