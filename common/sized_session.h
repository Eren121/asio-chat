#pragma once

#include "session.h"

namespace io
{

/**
 * One layer up `session`
 * The reader can deduce the size of the messages by inserting the size in front
 * of it.
 */
class sized_session
{
public:
    using header_size_type = int16_t;
    using on_read_handler = std::function<void(sized_session&, const char* bytes, size_t count)>;
    using on_error_handler = std::function<void(sized_session&, const std::error_code& error)>;
    
    explicit sized_session(asio::io_context& io_context, size_t buffer_size = session::default_buffer_size);
    explicit sized_session(tcp::socket socket, size_t buffer_size = session::default_buffer_size);
    
    tcp::endpoint local_endpoint() const { return m_session.local_endpoint(); }
    tcp::endpoint remote_endpoint() const { return m_session.remote_endpoint(); }
    void connect(const tcp::endpoint& server) { return m_session.connect(server); }
    void set_on_error(on_error_handler handler) { m_on_error_handler = std::move(handler); }
    void set_on_read(on_read_handler handler) { m_on_read_handler = std::move(handler); }
    
    void write(const char *bytes, size_t count);
    void read();
    
private:
    session m_session;
    on_read_handler m_on_read_handler;
    on_error_handler m_on_error_handler;
    bool m_in_header;
};

}