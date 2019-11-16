///////////////////////////////////////////////////////////////////////////////
// Writer.hpp
//
// Contains declaration of Writer class
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "shm_comm/common.hpp"

namespace shm {

/**
 * @brief Represents shared memory channel writer
 * @details It may be used to read data using shared memory.
 * The only requirement is to have a valid, existing channel instance, which
 * may be created using `Channel(...)`
 *
 * There are two types of buffer access functions: prefixed by `try_` and
 * not prefixed. First of them return status codes, second may throw. Thus
 * not-exceptional version is better suited for real-time applications.
 */
class Writer
{
public:
    /**
     * @brief Constructor
     * @details Opens shared memory writer
     * It opens connection to channels' writer.
     * Note that provided channel name must represent arleady existing
     * channel.
     *
     * If open fails, exception will be thrown
     *
     * @param name name of shared memory channel to connect
     * @return shared memory writer instance
     */
    explicit Writer(const ChannelName& channel_name);

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
    int buffer_size() const;

    ///////////////////////////////////////////////////////////////////////////////
    // Non-throwing buffer access functions
    ///////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Returns pointer to shared memory buffer. Non-throwing
     * @details It returns with an acquired valid pointer
     * to shared memory buffer. This function has to be called before
     * any call to `[try_]buffer_write` functions. This is because by getting
     * buffer's pointer we are signaling begin of write cycle.
     *
     * If this function fails, error code is returned.
     *
     * @param buffer pointer to shared memory buffer
     * @return Error code. =0 when no error,
     */
    int try_buffer_get(void** buffer);

    /**
     * @brief Signals end of write operation to the Reader. Non-throwing
     * @details Call to this function is required in order to signal
     * any reader, that data edition in Writer's has been ended and Reader
     * can safely read data from shared buffer.
     * You should call this function also to wake up any Reader,
     * which is waiting for read operation.
     *
     * If this function fails, error code is returned.
     *
     * @return Error code. =0 when no error.
     */
    int try_buffer_write();

    ///////////////////////////////////////////////////////////////////////////////
    // Throwing buffer access functions
    ///////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Returns pointer to shared memory buffer. Throwing
     * @details It returns with an acquired valid pointer
     * to shared memory buffer. This function has to be called before
     * any call to `[try_]buffer_write` functions. This is because by getting
     * buffer's pointer we are signaling begin of write cycle.
     *
     * If this function fails, exception is thrown.
     * On success, valid (non-null) pointer to memory is returned
     *
     * @return valid (non-null) pointer to shared memory buffer
     */
    void* buffer_get();

    /**
     * @brief Attempts to write
     * @details Call to this function is required in order to signal
     * any reader, that data edition in Writer's has been ended and Reader
     * can safely read data from shared buffer.
     * You should call this function also to wake up any Reader,
     * which is waiting for read operation.
     *
     * If this function fails, exception is thrown.
     */
    void buffer_write();

private:
    ///////////////////////////////////////////////////////////////////////////////
    // Private members
    ///////////////////////////////////////////////////////////////////////////////

    shm_writer_t* m_impl; //! Shared memory writer implementation
};

} // namespace shm
