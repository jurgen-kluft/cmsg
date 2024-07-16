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

        UNITTEST_TEST(process) {}
    }
}
UNITTEST_SUITE_END