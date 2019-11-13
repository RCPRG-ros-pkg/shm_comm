///////////////////////////////////////////////////////////////////////////////
// Channel_test.cpp
//
// Contains implementation of tests for Channel class
///////////////////////////////////////////////////////////////////////////////

#include <doctest.h>

#include <shm_comm/Channel.hpp>
#include <shm_comm/Reader.hpp>
#include <shm_comm/Writer.hpp>

///////////////////////////////////////////////////////////////////////////////
// Common variables
///////////////////////////////////////////////////////////////////////////////

const auto name = shm::ChannelName{"channel"};
const auto size = 123;
const auto readers = 5;

const auto other_name = shm::ChannelName{"other_channel"};
const auto other_size = 321;
const auto other_readers = 10;

///////////////////////////////////////////////////////////////////////////////
// Test cases
///////////////////////////////////////////////////////////////////////////////

TEST_CASE("Channels may be created")
{
    auto channel = shm::Channel::create(name, size, readers);

    SUBCASE("but there could be only one channel with same name")
    {
        CHECK_THROWS(shm::Channel::create(name, size, readers));
        CHECK_THROWS(shm::Channel::create(name, other_size, other_readers));
        CHECK_NOTHROW(shm::Channel::create(other_name, other_size, other_readers));
    }
}

TEST_CASE("Readers may be opened for given channel")
{
    auto channel = shm::Channel::create(name, size, readers);
    CHECK_NOTHROW(channel.open_reader());
}

TEST_CASE("Writers may be opened for given channel")
{
    auto channel = shm::Channel::create(name, size, readers);
    CHECK_NOTHROW(channel.open_writer());
}

TEST_CASE("Channels may be move-constructed")
{
    auto channel = shm::Channel::create(name, size, readers);
    auto new_channel = std::move(channel);

    SUBCASE("And we still could not create another with same name")
    {
        CHECK_THROWS(shm::Channel::create(name, other_size, other_readers));
    }

    SUBCASE("And we still can open reader for that channel")
    {
        CHECK_NOTHROW(new_channel.open_reader());
    }

    SUBCASE("And we still can open writer for that channel")
    {
        CHECK_NOTHROW(new_channel.open_writer());
    }
}
