///////////////////////////////////////////////////////////////////////////////
// Reader_test.cpp
//
// Contains implementation of tests for Reader class
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

TEST_CASE("Readers may be opened for existing channels")
{
    {
        const auto channel = shm::Channel::create(name, size, readers);
        CHECK_NOTHROW(shm::Reader::open(name));
    }

    {
        const auto other_channel = shm::Channel::create(other_name, other_size, other_readers);
        CHECK_NOTHROW(shm::Reader::open(other_name));
    }
}

TEST_CASE("Readers could not be opened for non-existing channels")
{
    {
        const auto channel = shm::Channel::create(name, size, readers);
        CHECK_THROWS(shm::Reader::open(other_name));
    }

    {
        const auto other_channel = shm::Channel::create(other_name, other_size, other_readers);
        CHECK_THROWS(shm::Reader::open(name));
    }
}


