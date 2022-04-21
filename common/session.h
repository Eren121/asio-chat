#pragma once

#include "common.h"
#include <vector>

namespace io
{

/**
 * Just wraps a socket.
 * All methods are asynchronous.
 * Can be both for storing a client in the server
 * or storing the session in the client.
 * Some functions (like connect) may be only useful on one side.
 */

class session
{
public:
    enum { default_buffer_size = 65536 };
    
    using on_read_handler = std::function<void(session&, const char* bytes, size_t count)>;
    using on_error_handler = std::function<void(session&, const std::error_code&)>;
    
    /**
     * @param buffer_size Maximum size for write() and read().
     */
    explicit session(asio::io_context& io_context, size_t buffer_size = default_buffer_size);
    explicit session(tcp::socket socket, size_t buffer_size = default_buffer_size);
    
    tcp::endpoint remote_endpoint() const;
    tcp::endpoint local_endpoint() const;
    
    size_t buffer_size() const { return m_buffer.size(); }
    
    void connect(const tcp::endpoint& server);
    
    /**
     * `bytes` is internally copied, so it may be destroyed after this call.
     */
    void write(const char* bytes, size_t count);
    
    /**
     * Store the data into the internal buffer so it doesn't need an output buffer,
     * and then pass it to the read handler.
     *
     * You MUST wait previous read() completion before calling a new one,
     * that is in read handler.
     */
    void read(size_t count);
    
    void set_on_error(on_error_handler handler);
    void set_on_read(on_read_handler handler);
    
private:
    void on_error(const std::error_code& error);
    void on_read(size_t count);
    
    void async_write();
    
    std::vector<char> m_buffer;
    
    on_error_handler m_on_error_handler;
    on_read_handler m_on_read_handler;
    tcp::socket m_socket;
    
    std::queue<std::vector<char>> m_outgoing;
};

}