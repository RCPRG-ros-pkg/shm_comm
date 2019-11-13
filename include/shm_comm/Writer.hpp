#pragma once

#include "shm_comm/common.hpp"

namespace shm {

/**
 * @brief Represents shared memory channel writer
 * @details It may be used to read data using shared memory.
 * The only requirement is to have a valid, existing channel instance, which
 * may be created using `Channel::create(...)`
 *
 * There are two types of buffer access functions: prefixed by `try_` and
 * not prefixed. First of them return status codes, second may throw. Thus
 * not-exceptional version is better suited for real-time applications.
 */
class Writer
{
public:
    /**
     * @brief Opens shared memory writer
     * @details It opens connection to channels' writer.
     * Note that provided channel name must represent arleady existing
     * channel.
     *
     * If open fails, exception will be thrown
     *
     * @param name name of shared memory channel to connect
     * @return shared memory writer instance
     */
    static Writer open(const ChannelName& channel_name);

    /**
     * @brief Destructor
     * @details Closes instance of shared memory writer
     */
    ~Writer();

    /**
     * @brief Move constructor
     * @details Passes ownership of writer instance to another object
     *
     * @param other
     */
    Writer(Writer&& other);

    /**
     * @brief Move assignment
     * @details Passes ownership of writer instance to another object
     *
     * @param other
     */
    Writer& operator=(Writer&& other);

    /**
     * @brief Copy constructor disabled
     */
    Writer(const Writer& other) = delete;

    /**
     * @brief Copy assignment disabled
     */
    Writer& operator=(const Writer& other) = delete;

    /**
     * @brief Gets size of shared memory buffer
     *
     * @return Size of shared memory buffer
     */
    int get_size();

    ///////////////////////////////////////////////////////////////////////////////
    // Non-throwing buffer access functions
    ///////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Returns pointer to shared memory buffer. Non-throwing
     * @details It returns with an acquired valid pointer
     * to shared memory buffer.
     *
     * If this function fails, error code is returned.
     *
     * @param buffer pointer to shared memory buffer
     * @return Error code. =0 when no error,
     */
    int try_buffer_get(void** buffer);

    /**
     * @brief Attempts to write prepared data right to the shared memory buffer
     * @details
     *
     * @return Error code. =0 when no error.
     */
    int try_buffer_write();

private:
    /**
     * @brief Constructor
     *
     * @param writer shared memory writer implementation
     */
    explicit Writer(shm_writer_t* impl);

    ///////////////////////////////////////////////////////////////////////////////
    // Private members
    ///////////////////////////////////////////////////////////////////////////////

    shm_writer_t* m_impl; //! Shared memory writer implementation
};

} // namespace shm
