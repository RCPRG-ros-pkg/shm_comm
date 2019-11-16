///////////////////////////////////////////////////////////////////////////////
// Reader.cpp
//
// Contains implementation of Reader class
///////////////////////////////////////////////////////////////////////////////

#include "shm_comm/Reader.hpp"

#include <cassert>
#include <cstdio>

#include <stdexcept>

namespace shm {

namespace {

shm_reader_t* open_reader(const ChannelName& channel_name)
{
    printf("[shm] Opening reader from channel '%s'\n", channel_name.data());

    shm_reader_t* reader_impl {nullptr};
    const auto result = shm_connect_reader(channel_name.data(), &reader_impl);
    if(result != 0)
    {
        throw std::runtime_error("Could not open shared memory reader, error: "
            + std::to_string(result));
    }

    assert(reader_impl != nullptr);
    return reader_impl;
}

} // namespace

Reader::Reader(const ChannelName& channel_name)
    :   m_impl{open_reader(channel_name)}
{
    printf("[shm] Reader %p initialized\n", (void*)m_impl);
}

Reader::~Reader()
{
    if(m_impl == nullptr)
    {
        // There is no reader to release
        return;
    }

    printf("[shm] Releasing reader %p\n", (void*)m_impl);
    const auto result = shm_release_reader(m_impl);
    if(result != 0)
    {
        printf("[shm] Could not release reader %p\n", (void*)m_impl);
    }
}

Reader::Reader(Reader&& other)
    :   m_impl(std::exchange(other.m_impl, nullptr))
{}

Reader& Reader::operator=(Reader&& other)
{
    assert(&other != this);

    m_impl = std::exchange(other.m_impl, nullptr);

    return *this;
}

int Reader::try_buffer_get(void** buffer)
{
    assert(m_impl != nullptr);
    assert(buffer != nullptr);

    return shm_reader_buffer_get(m_impl, buffer);
}

int Reader::try_buffer_wait(void** buffer)
{
    assert(m_impl != nullptr);
    assert(buffer != nullptr);

    return shm_reader_buffer_wait(m_impl, buffer);
}

int Reader::try_buffer_timedwait(const timespec& abstime, void **buffer)
{
    assert(m_impl != nullptr);
    assert(buffer != nullptr);

    return shm_reader_buffer_timedwait(m_impl, &abstime, buffer);
}

void* Reader::buffer_get()
{
    assert(m_impl != nullptr);

    void* buffer {nullptr};
    const auto result = shm_reader_buffer_get(m_impl, &buffer);
    if(result != 0)
    {
        throw std::runtime_error("Could not get shared memory reader buffer, error: "
            + std::to_string(result));
    }

    assert(buffer != nullptr);
    return buffer;
}

void* Reader::buffer_wait()
{
    assert(m_impl != nullptr);

    void* buffer {nullptr};
    const auto result = shm_reader_buffer_wait(m_impl, &buffer);
    if(result != 0)
    {
        throw std::runtime_error("Could not wait for shared memory reader buffer, error: "
            + std::to_string(result));
    }

    assert(buffer != nullptr);
    return buffer;
}

void* Reader::buffer_timedwait(const timespec& abstime)
{
    assert(m_impl != nullptr);

    void* buffer {nullptr};
    const auto result = shm_reader_buffer_timedwait(m_impl, &abstime, &buffer);
    if(result != 0)
    {
        throw std::runtime_error("Could not do timedwait for shared memory reader buffer, error: "
            + std::to_string(result));
    }

    assert(buffer != nullptr);
    return buffer;
}

} // namespace shm
