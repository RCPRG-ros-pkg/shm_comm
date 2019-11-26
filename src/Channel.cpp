///////////////////////////////////////////////////////////////////////////////
// Channel.cpp
//
// Contains implementation of Channel class
///////////////////////////////////////////////////////////////////////////////

#include "shm_comm/Channel.hpp"

#include <cassert>
#include <cstdio>
#include <cstring>

#include <stdexcept>

#include "shm_comm/shm_channel.h"
#include "shm_comm/shm_err.h"
#include "shm_comm/Reader.hpp"
#include "shm_comm/Writer.hpp"

namespace shm {

Channel::Channel(const char* name, int size, int readers, bool force)
{
    assert(name != nullptr);
    assert(size > 0);
    assert(readers > 0);

    printf("[shm] Creating channel '%s' with size %d bytes for %d readers...\n", name, size, readers);

    const auto result = shm_create_channel(name, size, readers, force);
    if(result != 0)
    {
        throw std::runtime_error("Could not create shared memory channel: "
            + std::string(shm_strerror(result)));
    }

    m_name.assign(name);
    assert(!m_name.empty());

    printf("[shm] Channel '%s' initialized\n", m_name.c_str());
}

Channel::~Channel()
{
    if(m_name.empty())
    {
        return;
    }

    printf("[shm] Removing channel '%s'\n", m_name.c_str());

    const auto result = shm_remove_channel(m_name.c_str());
    if(result != 0)
    {
        printf("[shm] Could not remove SHM channel: %s\n", shm_strerror(result));
    }
}

Channel::Channel(Channel&& other)
    :   m_name(std::move(other.m_name))
{}

Channel& Channel::operator=(Channel&& other)
{
    assert(&other != this);

    m_name = std::move(other.m_name);

    return *this;
}

Reader Channel::open_reader()
{
    assert(!m_name.empty());

    return Reader(m_name.c_str());
}

Writer Channel::open_writer()
{
    assert(!m_name.empty());

    return Writer(m_name.c_str());
}

} // namespace shm
