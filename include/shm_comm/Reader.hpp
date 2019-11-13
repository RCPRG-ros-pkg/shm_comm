#pragma once

#include "shm_comm/common.hpp"

namespace shm {

/**
 * @brief Represents shared memory channel reader
 * @details It may be used to read data using shared memory.
 * The only requirement is to have a valid, existing channel instance, which
 * may be created using `Channel::create(...)`
 *
 * Data reception may be done in two ways, by waiting for them, or directly by reading it.
 * Waiting for signal may be done using either `buffer_wait` or `buffer_timedwait`.
 * Additionally, you can directly obtain pointer to the buffer using `buffer_get` and
 * perform read on (hope) valid data.
 *
 * There are two types of buffer access functions: prefixed by `try_` and
 * not prefixed. First of them return status codes, second may throw. Thus
 * not-exceptional version is better suited for real-time applications.
 */
class Reader
{
public:
    /**
     * @brief Opens shared memory reader
     * @details It opens connection to one of remain channels' reader.
     * Note that provided channel name must represent arleady existing
     * channel, so better is to call Channel::open_reader(...) in terms of
     * safety (but not at all!)
     *
     * If open fails, exception will be thrown
     *
     * @param name name of shared memory channel to connect
     * @return shared memory reader instance
     */
    static Reader open(const ChannelName& channel_name);

    /**
     * @brief Destructor
     * @details Closes instance of shared memory reader
     */
    ~Reader();

    /**
     * @brief Move constructor
     * @details Passes ownership of reader instance to another object
     *
     * @param other
     */
    Reader(Reader&& other);

    /**
     * @brief Move assignment
     * @details Passes ownership of reader instance to another object
     *
     * @param other
     */
    Reader& operator=(Reader&& other);

    /**
     * @brief Copy constructor disabled
     */
    Reader(const Reader& other) = delete;

    /**
     * @brief Copy assignment disabled
     */
    Reader& operator=(const Reader& other) = delete;

    ///////////////////////////////////////////////////////////////////////////////
    // Non-throwing buffer access functions
    ///////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Returns pointer to shared memory buffer. Non-throwing
     * @details It immediately returns with a valid pointer
     * to shared memory buffer. But you should take care, that there could be
     * yet no data written by writer. When you would like to be sure, that
     * there is some data to receive, use one of waiting variants:
     * `_buffer_wait`, `_buffer_timedwait`.
     *
     * If this function fails, error code is returned.
     *
     * @param buffer pointer to shared memory buffer
     * @return Error code. =0 when no error,
     */
    int try_buffer_get(void** buffer);

    /**
     * @brief Infinitely waits for data in a shared memory buffer and returns pointer to it. Non-throwing.
     * @details
     *
     * @param buffer pointer to shared memory buffer
     * @return Error code. =0 when no error.
     */
    int try_buffer_wait(void **buffer);

    /**
     * @brief Waits for data in a shared memory buffer for some time and returns pointer to it. Non-throwing.
     * @details [long description]
     *
     * @param timespec absolute time until it should wait
     * @param buf pointer to shared memory buffer
     *
     * @return [description]
     */
    int try_buffer_timedwait(const struct timespec *abstime, void **buf);

private:
    /**
     * @brief Constructor
     *
     * @param reader shared memory reader implementation
     */
    explicit Reader(shm_reader_t* impl);

    ///////////////////////////////////////////////////////////////////////////////
    // Private members
    ///////////////////////////////////////////////////////////////////////////////

    shm_reader_t* m_impl; //! Shared memory reader implementation
};

} // namespace shm
