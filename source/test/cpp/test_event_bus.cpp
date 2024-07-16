#include "ccore/c_target.h"
#include "cbase/c_context.h"
#include "cbase/c_hbb.h"
#include "cmsg/c_event_bus.h"
#include "cmsg/test_allocator.h"

#include "cunittest/cunittest.h"

using namespace ncore;

UNITTEST_SUITE_BEGIN(event_bus)
{
    UNITTEST_FIXTURE(main)
    {
        UNITTEST_ALLOCATOR;

        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

        UNITTEST_TEST(create_destroy_bus)
        {
            nevent::event_bus_t* bus = nevent::create_event_bus(Allocator);
            CHECK_NOT_NULL(bus);
            nevent::destroy_event_bus(Allocator, bus);
        }

        struct TestEvent
        {
            int   a;
            float b;
        };

        UNITTEST_TEST(create_destroy_bus_and_event_channel)
        {
            nevent::event_bus_t* bus = nevent::create_event_bus(Allocator);
            CHECK_NOT_NULL(bus);

            ncore::Callback1<void, TestEvent const&> delegate;
            nevent::register_event_subscriber(bus, delegate);

            // ...

            nevent::unregister_event_subscriber(bus, delegate);
            nevent::destroy_event_bus(Allocator, bus);
        }
    }

    UNITTEST_FIXTURE(subscribe_post_process)
    {
        UNITTEST_ALLOCATOR;

        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

        struct TestEvent
        {
            int   a;
            float b;
        };

        static TestEvent sTestEvent;
        void             ProcessEvent(TestEvent const& a)
        {
            sTestEvent.a += a.a;
            sTestEvent.b += a.b;
        }

        UNITTEST_TEST(subscribe_single_event)
        {
            nevent::event_bus_t* bus = nevent::create_event_bus(Allocator);
            CHECK_NOT_NULL(bus);

            ncore::Callback1<void, TestEvent const&> delegate;
            delegate = ProcessEvent;

            nevent::register_event_subscriber(bus, delegate);

            sTestEvent.a = 0;
            sTestEvent.b = 0.0f;

            TestEvent event;
            event.a = 1;
            event.b = 2.0f;
            nevent::post_event(bus, event);

            process_events(bus);

            CHECK_EQUAL(1, sTestEvent.a);
            CHECK_EQUAL(2.0f, sTestEvent.b);

            event.a = 3;
            event.b = 4.0f;
            nevent::post_event(bus, event);

            process_events(bus);

            CHECK_EQUAL(1 + 3, sTestEvent.a);
            CHECK_EQUAL(2.0f + 4.0f, sTestEvent.b);

            nevent::unregister_event_subscriber(bus, delegate);
            nevent::destroy_event_bus(Allocator, bus);
        }

        UNITTEST_TEST(subscribe_multiple_events)
        {
            nevent::event_bus_t* bus = nevent::create_event_bus(Allocator);
            CHECK_NOT_NULL(bus);

            ncore::Callback1<void, TestEvent const&> delegate;
            delegate = ProcessEvent;

            nevent::register_event_subscriber(bus, delegate);

            sTestEvent.a = 0;
            sTestEvent.b = 0.0f;

            TestEvent event;
            event.a = 1;
            event.b = 2.0f;
            nevent::post_event(bus, event);

            event.a = 3;
            event.b = 4.0f;
            nevent::post_event(bus, event);

            process_events(bus);

            CHECK_EQUAL(1 + 3, sTestEvent.a);
            CHECK_EQUAL(2.0f + 4.0f, sTestEvent.b);

            nevent::unregister_event_subscriber(bus, delegate);
            nevent::destroy_event_bus(Allocator, bus);
        }
    }

    UNITTEST_FIXTURE(multi_subscribe_multi_post_process)
    {
        UNITTEST_ALLOCATOR;

        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

        struct TestEventOne
        {
            int   a;
            float b;
        };
        static TestEventOne sTestEventOne;
        void             ProcessEventOne(TestEventOne const& a)
        {
            sTestEventOne.a += a.a;
            sTestEventOne.b += a.b;
        }

        struct TestEventTwo
        {
            int   a;
            float b;
            double c;
        };
        static TestEventTwo sTestEventTwo;
        void             ProcessEventTwo(TestEventTwo const& a)
        {
            sTestEventTwo.a += a.a;
            sTestEventTwo.b += a.b;
            sTestEventTwo.c += a.c;
        }

        UNITTEST_TEST(subscribe_single_event)
        {
            nevent::event_bus_t* bus = nevent::create_event_bus(Allocator);
            CHECK_NOT_NULL(bus);

            ncore::Callback1<void, TestEventOne const&> delegateOne;
            delegateOne = ProcessEventOne;
            nevent::register_event_subscriber(bus, delegateOne);

            ncore::Callback1<void, TestEventTwo const&> delegateTwo;
            delegateTwo = ProcessEventTwo;
            nevent::register_event_subscriber(bus, delegateTwo);

            sTestEventOne.a = 0;
            sTestEventOne.b = 0.0f;

            sTestEventTwo.a = 0;
            sTestEventTwo.b = 0.0f;
            sTestEventTwo.c = 0.0;

            TestEventOne eventOne;
            eventOne.a = 1;
            eventOne.b = 2.0f;
            nevent::post_event(bus, eventOne);
            
            TestEventTwo eventTwo;
            eventTwo.a = 1;
            eventTwo.b = 2.0f;
            eventTwo.c = 3.0;
            nevent::post_event(bus, eventTwo);

            process_events(bus);

            CHECK_EQUAL(1, sTestEventOne.a);
            CHECK_EQUAL(2.0f, sTestEventOne.b);

            CHECK_EQUAL(1, sTestEventTwo.a);
            CHECK_EQUAL(2.0f, sTestEventTwo.b);
            CHECK_EQUAL(3.0, sTestEventTwo.c);

            eventOne.a = 3;
            eventOne.b = 4.0f;
            nevent::post_event(bus, eventOne);

            eventTwo.a = 3;
            eventTwo.b = 4.0f;
            eventTwo.c = 5.0f;
            nevent::post_event(bus, eventTwo);

            process_events(bus);

            CHECK_EQUAL(1 + 3, sTestEventOne.a);
            CHECK_EQUAL(2.0f + 4.0f, sTestEventOne.b);

            CHECK_EQUAL(1 + 3, sTestEventTwo.a);
            CHECK_EQUAL(2.0f + 4.0f, sTestEventTwo.b);
            CHECK_EQUAL(3.0 + 5.0, sTestEventTwo.c);

            nevent::unregister_event_subscriber(bus, delegateOne);
            nevent::unregister_event_subscriber(bus, delegateTwo);
            nevent::destroy_event_bus(Allocator, bus);
        }

    }
}
UNITTEST_SUITE_END