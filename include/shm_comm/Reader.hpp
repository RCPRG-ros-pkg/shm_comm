///////////////////////////////////////////////////////////////////////////////
// Reader.hpp
//
// Contains declaration of Reader class
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "shm_comm/common.hpp"

namespace shm {

/**
 * @brief Represents shared memory channel reader
 * @details It may be used to read data using shared memory.
 * The only requirement is to have a valid, existing channel instance, which
 * may be created using `Channel(...)`
 *
 * There are two types of buffer access functions: prefixed by `try_` and
 * not prefixed. First of them return status codes, second may throw. Thus
 * not-exceptional version is better suited for real-time applications.

 * Data reception may be done in two ways, by waiting for them, or directly by reading it.
 * Waiting for signal may be done using either `buffer_wait` or `buffer_timedwait`.
 * Additionally, you can directly obtain pointer to the buffer using `buffer_get` and
 * perform read on (hope) existing and valid data.
 *
 * According wait-related functions. Note that after creating the channel
 * you should perform first a write-read process using Writer-Reader.
 * It is needed in order to properly initialize shared memory buffer
 * structure. Without this, an attempt to wait for data by Reader will end
 * with an error. So, to overcome this problem you should do something like this:
 *
 * ```c++
 *  const auto channel = shm::Channel(channel_name, size, nreaders);
 *  auto writer = shm::Writer(channel_name);
 *  auto reader = shm::Reader(channel_name);
 *
 *  // Perform first write-read operation
 *  // It is needed to initialize shm buffers
 *  writer.buffer_get();
 *  writer.buffer_write();
 *  reader.buffer_get();
 *
 *  // Now you can safely wait for data:
 *  reader.buffer_timedwait(...);
 * ```
 */
class Reader
{
public:
    /**
     * @brief Constructor
     * @details Opens shared memory reader
     * It opens connection to one of remain channels' reader.
     * Note that provided channel name must represent arleady existing
     * channel, so better is to call Channel::open_reader(...) in terms of
     * safety (but not at all!)
     *
     * If open fails, exception will be thrown
     *
     * @param name name of shared memory channel to connect
     * @return shared memory reader instance
     */
    explicit Reader(const ChannelName& channel_name);

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
     * @param abstime absolute time until it should wait (specified by steady clock)
     * @param buf pointer to shared memory buffer
     *
     * @return Error code. =0 when no error.
     */
    int try_buffer_timedwait(const timespec& abstime, void **buf);

    ///////////////////////////////////////////////////////////////////////////////
    // Throwing buffer access functions
    ///////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Returns pointer to shared memory buffer. Throwing
     * @details It immediately returns with a valid pointer
     * to shared memory buffer. But you should take care, that there could be
     * yet no data written by writer. When you would like to be sure, that
     * there is some data to receive, use one of waiting variants:
     * `_buffer_wait`, `_buffer_timedwait`.
     *
     * If this function fails, exception will be thrown
     *
     * @param buffer pointer to shared memory buffer
     */
    void* buffer_get();

    /**
     * @brief Infinitely waits for data in a shared memory buffer and returns pointer to it. Throwing
     * @details It internally executes non-timed wait (endless), so use this function with care.
     * Note, that before first wait you should ensure proper channel initialization, by executing
     * dummy first-read sequence. In other case, error will be reported about no data (ever) in buffer.
     *
     * If this function fails, exception will be thrown
     *
     * @return pointer to shared memory buffer
     */
    void* buffer_wait();

    /**
     * @brief Waits for data in a shared memory buffer for some time and returns pointer to it. Non-throwing.
     * @details This function is a safer replacement against `[try_]buffer_wait()` functions, because program
     * will not block indifinetely, but for specified amount of time.
     * Note, that before first wait you should ensure proper channel initialization, by executing
     * dummy first-read sequence. In other case, error will be reported about no data (ever) in buffer.
     *
     * If this function fails, exception will be thrown
     *
     * @param abstime absolute time until it should wait (specified by steady clock)
     * @return pointer to shared memory buffer
     */
    void* buffer_timedwait(const timespec& abstime);

private:
    ///////////////////////////////////////////////////////////////////////////////
    // Private members
    ///////////////////////////////////////////////////////////////////////////////

    shm_reader_t* m_impl; //! Shared memory reader implementation
};

} // namespace shm
