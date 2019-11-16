///////////////////////////////////////////////////////////////////////////////
// Writer_test.cpp
//
// Contains implementation of tests for Writer class
///////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <ctime>
#include <cstdlib>

#include <doctest.h>

#include <shm_comm/Channel.hpp>
#include <shm_comm/Writer.hpp>
#include <shm_comm/Reader.hpp>

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

TEST_CASE("Writers may be opened for existing channels")
{
    {
        const auto channel = shm::Channel(channel_name, size, nreaders);
        CHECK_NOTHROW(shm::Writer::open(channel_name));
    }

    {
        const auto other_channel = shm::Channel(other_channel_name, other_size, other_nreaders);
        CHECK_NOTHROW(shm::Writer::open(other_channel_name));
    }
}

TEST_CASE("Writers could not be opened for non-existing channels")
{
    {
        const auto channel = shm::Channel(channel_name, size, nreaders);
        CHECK_THROWS(shm::Writer::open(other_channel_name));
    }

    {
        const auto other_channel = shm::Channel(other_channel_name, other_size, other_nreaders);
        CHECK_THROWS(shm::Writer::open(channel_name));
    }
}

TEST_CASE("There is no (in theory) limit at number of writers")
{
    const auto channel = shm::Channel(channel_name, size, nreaders);

    std::vector<shm::Writer> writers;
    for(auto i = 0; i < 100; ++i)
    {
        CHECK_NOTHROW(writers.push_back(shm::Writer::open(channel_name)));
    }
}

TEST_CASE("Non-null pointer to buffer may be obtained from a valid writer")
{
    const auto channel = shm::Channel(channel_name, size, nreaders);
    auto writer = shm::Writer::open(channel_name);

    const auto buffer = writer.buffer_get();
    CHECK(buffer != nullptr);

    SUBCASE("Further requests for buffer pointer are same")
    {
        for(auto i = 0; i < 10; ++i)
        {
            const auto next_buffer = writer.buffer_get();
            CHECK(next_buffer == buffer);
        }
    }
}

TEST_CASE("Writer's buffer will have size specified by Channel")
{
    const auto channel = shm::Channel(channel_name, size, nreaders);
    const auto writer = shm::Writer::open(channel_name);
    CHECK(writer.buffer_size() == size);

    const auto other_channel = shm::Channel(other_channel_name, other_size, other_nreaders);
    const auto other_writer = shm::Writer::open(other_channel_name);
    CHECK(other_writer.buffer_size() == other_size);
}

TEST_CASE("Editing of writer's buffer is allowed")
{
    const auto channel = shm::Channel(channel_name, size, nreaders);
    auto writer = shm::Writer::open(channel_name);
    REQUIRE(writer.buffer_size() == size);

    auto buffer = writer.buffer_get();
    REQUIRE(buffer != nullptr);

    srand(time(NULL));
    char* data = (char*)buffer;
    for(auto i = 0; i < size; ++i)
    {
        const auto value = static_cast<char>(rand() % 255);
        data[i] = value;
    }
}

TEST_CASE("After edition of data, writer's buffer may be written (commited)")
{
    const auto channel = shm::Channel(channel_name, size, nreaders);
    auto writer = shm::Writer::open(channel_name);
    REQUIRE(writer.buffer_size() == size);

    auto buffer = writer.buffer_get();
    REQUIRE(buffer != nullptr);

    char* data = reinterpret_cast<char*>(buffer);
    data[0] = 'a';
    data[1] = 'b';
    data[2] = 'c';
    CHECK_NOTHROW(writer.buffer_write());

    SUBCASE("And written data will be imediately received by reader")
    {
        auto reader = shm::Reader::open(channel_name);

        auto read_buffer = reader.buffer_wait();
        REQUIRE(read_buffer != nullptr);

        auto read_data = reinterpret_cast<char*>(read_buffer);
        CHECK(read_data[0] == 'a');
        CHECK(read_data[1] == 'b');
        CHECK(read_data[2] == 'c');

        SUBCASE("But not written (commited) data will also be visible by reader")
        {
            data[0] = 'x';
            data[1] = 'y';
            data[2] = 'z';

            // Previous data must persist
            CHECK(read_data[0] == 'x');
            CHECK(read_data[1] == 'y');
            CHECK(read_data[2] == 'z');
        }
    }

    SUBCASE("And buffer's size does not change")
    {
        CHECK(writer.buffer_size() == size);
    }
}

TEST_CASE("Multiple writes (commits), without Reader's readings, are allowed")
{
    const auto channel = shm::Channel(channel_name, size, nreaders);
    auto writer = shm::Writer::open(channel_name);
    REQUIRE(writer.buffer_size() == size);

    for(auto i = 0; i < 10; ++i)
    {
        auto buffer = writer.buffer_get();
        REQUIRE(buffer != nullptr);

        // We can do multiple writes without Reader's action
        CHECK_NOTHROW(writer.buffer_write());
    }
}

TEST_CASE("Multiple writes (commits) with according Reader's readings are allowed")
{
    const auto channel = shm::Channel(channel_name, size, nreaders);
    auto writer = shm::Writer::open(channel_name);
    REQUIRE(writer.buffer_size() == size);
    REQUIRE_NOTHROW(writer.buffer_get());
    REQUIRE_NOTHROW(writer.buffer_write());

    auto reader = shm::Reader::open(channel_name);
    for(auto i = 0; i < 10; ++i)
    {
        // After Reader's actions...
        REQUIRE_NOTHROW(reader.buffer_wait());

        // ... we should be able to do next write
        CHECK_NOTHROW(writer.buffer_get());
        CHECK_NOTHROW(writer.buffer_write());
    }
}

