///////////////////////////////////////////////////////////////////////////////
// Writer.cpp
//
// Contains implementation of Writer class
///////////////////////////////////////////////////////////////////////////////

#include "shm_comm/Writer.hpp"

#include <cassert>
#include <cstdio>
#include <ctime>

#include <stdexcept>
#include <utility>

#include "shm_comm/shm_channel.h"

namespace shm {

namespace {

shm_writer_t* open_writer(const char* channel_name)
{
    printf("[shm] Opening writer from channel '%s'\n", channel_name);

    shm_writer_t* writer_impl {nullptr};
    const auto result = shm_connect_writer(channel_name, &writer_impl);
    if(result != 0)
    {
        throw std::runtime_error("Could not open shared memory writer: "
            + std::string(shm_strerror(result)));
    }

    assert(writer_impl != nullptr);
    return writer_impl;
}

} // namespace

Writer::Writer(const char* channel_name)
    :   m_impl{open_writer(channel_name)}
{
    printf("[shm] Writer %p initialized\n", (void*)m_impl);
}

Writer::~Writer()
{
    if(m_impl == nullptr)
    {
        // There is no writer to release
        return;
    }

    printf("[shm] Releasing writer %p\n", (void*)m_impl);
    const auto result = shm_release_writer(m_impl);
    if(result != 0)
    {
        printf("[shm] Could not release writer: %s\n", shm_strerror(result));
    }
}

Writer::Writer(Writer&& other)
    :   m_impl(std::exchange(other.m_impl, nullptr))
{}

Writer& Writer::operator=(Writer&& other)
{
    assert(&other != this);

    m_impl = std::exchange(other.m_impl, nullptr);

    return *this;
}

int Writer::buffer_size() const
{
    assert(m_impl != nullptr);

    return shm_writer_get_size(m_impl);
}

int Writer::try_buffer_get(void** buffer)
{
    assert(m_impl != nullptr);
    assert(buffer != nullptr);

    return shm_writer_buffer_get(m_impl, buffer);
}

int Writer::try_buffer_write()
{
    assert(m_impl != nullptr);

    return shm_writer_buffer_write(m_impl);
}

void* Writer::buffer_get()
{
    assert(m_impl != nullptr);

    void* buffer {nullptr};
    const auto result = shm_writer_buffer_get(m_impl, &buffer);
    if(result != 0)
    {
        throw std::runtime_error("Could not get shared memory writer buffer: "
            + std::string(shm_strerror(result)));
    }

    assert(buffer != nullptr);
    return buffer;
}

void Writer::buffer_write()
{
    assert(m_impl != nullptr);

    const auto result = shm_writer_buffer_write(m_impl);
    if(result != 0)
    {
        throw std::runtime_error("Could not write shared memory buffer: "
            + std::string(shm_strerror(result)));
    }
}

} // namespace shm
