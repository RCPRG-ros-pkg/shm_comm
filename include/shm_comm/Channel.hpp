///////////////////////////////////////////////////////////////////////////////
// Channel.hpp
//
// Contains declaration of Channel class
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <experimental/optional>

#include "shm_comm/common.hpp"

namespace shm {

///////////////////////////////////////////////////////////////////////////////
// Forward declarations
///////////////////////////////////////////////////////////////////////////////

class Reader;
class Writer;

///////////////////////////////////////////////////////////////////////////////
// Public classes
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Represents instance of shared memory channel
 * @details Channel is main part in IPC using shared memory.
 * This implementation is based of linux' SHM API and is suited
 *  for real-time applications.
 * With single channel you can open unlimited number of writers
 *  and limited number of readers. Each channel have limited
 *  size, specified at creation time.
 *
 * Usage example:
 *
 * ```c++
 *  const auto channel_name = "ethercat_status";
 *  const auto channel_size = 1536;
 *  const auto channel_readers = 1;
 *  auto channel = shm::create(channel_name, channel_size, channel_readers);
 *  auto reader = channel.open_reader();
 *  // ...
 *  reader.buffer_wait();
 *  const auto buffer = reader.buffer_get();
 *  // ...
 * ```
 */
class Channel
{
public:
    /**
     * @brief Creates shared memory channel
     * @details Though number of readers are fixed to some value (@param readers),
     *  number of writes are not limited (infinity).
     * As long as received Channel instance exist, this channel will remain.
     *
     * If creation of that channel fails, exception will be thrown
     *
     * @param name name of shared memory channel
     * @param size size in bytes of shared memory channel
     * @param readers number of simultaneous readers
     * @param force
     * @return successfully created or valid channel
     */
    static Channel create(const ChannelName& name, int size, int readers, bool force = false);

    /**
     * @brief Destructor
     * @details Removes (if still exists) instance of shared memory channel
     */
    ~Channel();

    /**
     * @brief Move constructor
     * @details Transfers ownership of Channel instance
     */
    Channel(Channel&& other);

    /**
     * @brief Move assignment
     * @details Transfers ownership of Channel instance
     */
    Channel& operator=(Channel&& other);

    /**
     * @brief Copy constructor disabled
     */
    Channel(const Channel& other) = delete;

    /**
     * @brief Copy assignment disabled
     */
    Channel& operator=(const Channel& other) = delete;

    /**
     * @brief Opens connection to shared memory reader
     * @details It is a shorthand for `Reader::open(name)`, but gives us more
     * protection about unexisting channel (but not at all!)
     *
     * If opening of reader fails, exception will be thrown
     *
     * @return successfully connected reader
     */
    Reader open_reader();

    /**
     * @brief Opens connection to shared memory writer
     * @details It is a shorthand for `Reader::open(name)`, but gives us more
     * protection about unexisting channel (but not at all!)
     *
     * If opening of writer fails, exception will be thrown
     *
     * @return successfully connected reader
     */
    Writer open_writer();

private:
    /**
     * @brief Constructor
     * @details Just initializes member variables
     *
     * @param name name of shared memory channel
     * @param whether this is an owner of the channel
     */
    Channel(const ChannelName& name);

    ///////////////////////////////////////////////////////////////////////////////
    // Private members
    ///////////////////////////////////////////////////////////////////////////////

    std::experimental::optional<ChannelName> m_name;
};

} // namespace shm
