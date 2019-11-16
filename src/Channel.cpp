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

#include "shm_comm/Reader.hpp"
#include "shm_comm/Writer.hpp"

namespace shm {

Channel::Channel(const ChannelName& name, int size, int readers, bool force)
{
    assert(size > 0);
    assert(readers > 0);

    printf("[shm] Creating channel '%s' with size %d bytes for %d readers...\n", name.data(), size, readers);

    const auto result = shm_create_channel(name.data(), size, readers, force);
    if(result != 0)
    {
        throw std::runtime_error("Could not create shared memory channel, error: "
            + std::to_string(result));
    }

    m_name.emplace();
    strncpy(m_name->data(), name.data(), name.size());

    printf("[shm] Channel '%s' initialized\n", m_name->data());
}

Channel::~Channel()
{
    if(!m_name)
    {
        return;
    }

    printf("[shm] Removing channel '%s'\n", m_name->data());

    const auto result = shm_remove_channel(m_name->data());
    if(result != 0)
    {
        printf("[shm] Could not remove SHM channel, error: %d\n", result);
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
    assert(m_name);

    return Reader::open(*m_name);
}

Writer Channel::open_writer()
{
    assert(m_name);

    return Writer::open(*m_name);
}

} // namespace shm
