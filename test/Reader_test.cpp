///////////////////////////////////////////////////////////////////////////////
// Reader_test.cpp
//
// Contains implementation of tests for Reader class
///////////////////////////////////////////////////////////////////////////////

#include <csignal>

#include <vector>
#include <thread>

#include <pthread.h>

#include <doctest.h>

#include <shm_comm/Channel.hpp>
#include <shm_comm/Reader.hpp>
#include <shm_comm/Writer.hpp>

using namespace std::chrono_literals;

///////////////////////////////////////////////////////////////////////////////
// Common variables
///////////////////////////////////////////////////////////////////////////////

const auto channel_name = shm::ChannelName{"channel"};
const auto size = 123;
const auto nreaders = 5;

const auto other_channel_name = shm::ChannelName{"other_channel"};
const auto other_size = 321;
const auto other_nreaders = 10;

///////////////////////////////////////////////////////////////////////////////
// Test cases
///////////////////////////////////////////////////////////////////////////////

TEST_CASE("Readers may be opened for existing channels")
{
    {
        const auto channel = shm::Channel::create(channel_name, size, nreaders);
        CHECK_NOTHROW(shm::Reader::open(channel_name));
    }

    {
        const auto other_channel = shm::Channel::create(other_channel_name, other_size, other_nreaders);
        CHECK_NOTHROW(shm::Reader::open(other_channel_name));
    }
}

TEST_CASE("Readers could not be opened for non-existing channels")
{
    {
        const auto channel = shm::Channel::create(channel_name, size, nreaders);
        CHECK_THROWS(shm::Reader::open(other_channel_name));
    }

    {
        const auto other_channel = shm::Channel::create(other_channel_name, other_size, other_nreaders);
        CHECK_THROWS(shm::Reader::open(channel_name));
    }
}

TEST_CASE("There could be only opened as much readers as specified in the channel")
{
    const auto channel = shm::Channel::create(channel_name, size, nreaders);

    std::vector<shm::Reader> readers;
    for(auto i = 0; i < nreaders; ++i)
    {
        CHECK_NOTHROW(readers.push_back(shm::Reader::open(channel_name)));
    }

    SUBCASE("Opening an another reader should fail")
    {
        CHECK_THROWS(shm::Reader::open(channel_name));
    }
}

TEST_CASE("Non-null pointer to buffer may be obtained after writer's write to that buffer")
{
    const auto channel = shm::Channel::create(channel_name, size, nreaders);
    auto writer = shm::Writer::open(channel_name);
    auto reader = shm::Reader::open(channel_name);

    for(auto i = 0; i < 10; ++i)
    {
        REQUIRE_NOTHROW(writer.buffer_get());
        REQUIRE_NOTHROW(writer.buffer_write());

        void* buffer;
        CHECK_NOTHROW(buffer = reader.buffer_get());
        REQUIRE(buffer != nullptr);
    }
}

TEST_CASE("Pointer to buffer will be not obtained, when writer didn't do any write to that buffer")
{
    const auto channel = shm::Channel::create(channel_name, size, nreaders);
    auto reader = shm::Reader::open(channel_name);

    CHECK_THROWS(reader.buffer_get());
}

TEST_CASE("After Writer's write, only first Reader's buffer get is allowed")
{
    const auto channel = shm::Channel::create(channel_name, size, nreaders);
    auto writer = shm::Writer::open(channel_name);
    auto reader = shm::Reader::open(channel_name);

    REQUIRE_NOTHROW(writer.buffer_get());
    REQUIRE_NOTHROW(writer.buffer_write());

    // First request is OK
    REQUIRE_NOTHROW(reader.buffer_get());

    // There is "no more data", so we cannot get any
    for(auto i = 0; i < 10; ++i)
    {
        CHECK_THROWS(reader.buffer_get());
    }
}

TEST_CASE("Reader could wait infinitely for write to buffer from Writer")
{
    const auto channel = shm::Channel::create(channel_name, size, nreaders);
    auto writer = shm::Writer::open(channel_name);
    auto reader = shm::Reader::open(channel_name);

    // Perform first write-read operation
    // It is needed to initialize shm buffers
    REQUIRE_NOTHROW(writer.buffer_get());
    REQUIRE_NOTHROW(writer.buffer_write());
    REQUIRE_NOTHROW(reader.buffer_get());

    SUBCASE("When Writer do write before Reader go wait, Reader will end waiting imediately")
    {
        // Perform write operation
        REQUIRE_NOTHROW(writer.buffer_get());
        REQUIRE_NOTHROW(writer.buffer_write());

        auto waiter = std::thread([&reader]() {
            void* buffer {nullptr};
            CHECK_NOTHROW(buffer = reader.buffer_wait());
            CHECK(buffer != nullptr);
        });

        // And wait for Reader's gracefully end of waiting
        REQUIRE_NOTHROW(waiter.join());
    }

    SUBCASE("When Writer do write (possibly) after Reader go wait, Reader will end waiting")
    {
        auto waiter = std::thread([&reader]() {
            void* buffer {nullptr};
            CHECK_NOTHROW(buffer = reader.buffer_wait());
            CHECK(buffer != nullptr);
        });

        // Perform write operation
        REQUIRE_NOTHROW(writer.buffer_get());
        REQUIRE_NOTHROW(writer.buffer_write());

        // And wait for Reader's gracefully end of waiting
        REQUIRE_NOTHROW(waiter.join());
    }

    // SUBCASE("When Writer doesn't do any write, Reader will wait endless")
    // {
    //     REQUIRE_NOTHROW(writer.buffer_get());
    //     REQUIRE_NOTHROW(writer.buffer_write());
    //     REQUIRE_NOTHROW(reader.buffer_wait());

    //     auto waiter = std::thread([&reader]() {
    //         // We have to be prepared for signal in order to exit endless wait
    //         sigset_t set;
    //         sigemptyset(&set);
    //         sigaddset(&set, SIGINT);
    //         REQUIRE(pthread_sigmask(SIG_BLOCK, &set, NULL) == 0);

    //         void* buffer {nullptr};
    //         CHECK_NOTHROW(buffer = reader.buffer_wait());
    //         CHECK(buffer == nullptr);
    //     });

    //     // Wait a bit
    //     REQUIRE_NOTHROW(std::this_thread::sleep_for(500us));

    //     printf("about to kill\n");

    //     // Send signal to waiter in order to interrupt endless wait
    //     REQUIRE(pthread_kill(waiter.native_handle(), SIGINT) == 0);

    //     printf("killed\n");

    //     // And wait for interrupted end of waiting
    //     REQUIRE_NOTHROW(waiter.join());
    // }
}

TEST_CASE("Reader could timedwait for write to buffer from Writer")
{
    const auto channel = shm::Channel::create(channel_name, size, nreaders);
    auto writer = shm::Writer::open(channel_name);
    auto reader = shm::Reader::open(channel_name);
    const auto TIMEOUT_SECS = 1;

    // Perform first write-read operation
    // It is needed to initialize shm buffers
    REQUIRE_NOTHROW(writer.buffer_get());
    REQUIRE_NOTHROW(writer.buffer_write());
    REQUIRE_NOTHROW(reader.buffer_get());

    SUBCASE("When Writer do write before Reader go timedwait, Reader will end timedwaiting imediately")
    {
        // Perform write operation before Reader's wait
        REQUIRE_NOTHROW(writer.buffer_get());
        REQUIRE_NOTHROW(writer.buffer_write());

        // Launch Reader's wait in another thread
        auto waiter = std::thread([&reader]() {
            // Prepare timeout
            timespec ts;
            clock_gettime(CLOCK_MONOTONIC, &ts);
            ts.tv_sec += TIMEOUT_SECS;

            // Wait for write
            void* buffer {nullptr};
            CHECK_NOTHROW(buffer = reader.buffer_timedwait(ts));
            CHECK(buffer != nullptr);
        });

        // And wait for Reader's gracefully end of waiting
        REQUIRE_NOTHROW(waiter.join());
    }

    SUBCASE("When Writer do write (possibly) after Reader go timedwait, Reader will end timedwaiting imediately")
    {
        // Launch Reader's wait in another thread
        auto waiter = std::thread([&reader]() {
            // Prepare timeout
            timespec ts;
            clock_gettime(CLOCK_MONOTONIC, &ts);
            ts.tv_sec += TIMEOUT_SECS;

            // Wait for write
            void* buffer {nullptr};
            CHECK_NOTHROW(buffer = reader.buffer_timedwait(ts));
            CHECK(buffer != nullptr);
        });

        // Perform write operation (possibly) after Reader's wait
        REQUIRE_NOTHROW(writer.buffer_get());
        REQUIRE_NOTHROW(writer.buffer_write());

        // And wait for Reader's gracefully end of waiting
        REQUIRE_NOTHROW(waiter.join());
    }

    SUBCASE("When Writer doesn't do any write, Reader will end with a timeout")
    {
        // Launch Reader's wait in another thread
        auto waiter = std::thread([&reader]() {
            // Prepare timeout
            timespec ts;
            const auto result = clock_gettime(CLOCK_MONOTONIC, &ts);
            REQUIRE(result == 0);
            ts.tv_sec += TIMEOUT_SECS;

            // Wait for write
            void* buffer {nullptr};
            CHECK_THROWS(buffer = reader.buffer_timedwait(ts));
            CHECK(buffer == nullptr);
        });

        // And wait for Reader's gracefully end of waiting
        REQUIRE_NOTHROW(waiter.join());
    }

    SUBCASE("After failed timedwait, Reader could timedwait again")
    {
        // Launch Reader's wait in another thread
        auto waiter = std::thread([&reader]() {
            // Prepare timeout
            timespec ts;
            const auto result = clock_gettime(CLOCK_MONOTONIC, &ts);
            REQUIRE(result == 0);
            ts.tv_sec += TIMEOUT_SECS;

            // Wait for write
            void* buffer {nullptr};
            CHECK_THROWS(buffer = reader.buffer_timedwait(ts));
            CHECK(buffer == nullptr);
        });

        // And wait for Reader's gracefully end of waiting
        REQUIRE_NOTHROW(waiter.join());

        SUBCASE("Can again wait for no data from writer")
        {
            // Launch another Reader's wait in another thread
            auto another_waiter = std::thread([&reader]() {
                // Prepare timeout
                timespec ts;
                const auto result = clock_gettime(CLOCK_MONOTONIC, &ts);
                REQUIRE(result == 0);
                ts.tv_sec += TIMEOUT_SECS;

                // Wait for write
                void* buffer {nullptr};
                CHECK_THROWS(buffer = reader.buffer_timedwait(ts));
                CHECK(buffer == nullptr);
            });

            // Wait for thread end
            REQUIRE_NOTHROW(another_waiter.join());
        }

        SUBCASE("Can again wait for data from writer")
        {
            // Launch another Reader's wait in another thread
            auto another_waiter = std::thread([&reader]() {
                // Prepare timeout
                timespec ts;
                const auto result = clock_gettime(CLOCK_MONOTONIC, &ts);
                REQUIRE(result == 0);
                ts.tv_sec += TIMEOUT_SECS;

                // Wait for write
                void* buffer {nullptr};
                CHECK_NOTHROW(buffer = reader.buffer_timedwait(ts));
                CHECK(buffer != nullptr);
            });

            // Perform write operation (possibly) after Reader's wait
            REQUIRE_NOTHROW(writer.buffer_get());
            REQUIRE_NOTHROW(writer.buffer_write());

            // Wait for thread end
            REQUIRE_NOTHROW(another_waiter.join());
        }
    }
}
