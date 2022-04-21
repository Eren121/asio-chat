#include "session.h"

namespace io
{

session::session(asio::io_context& io_context, size_t buffer_size)
    : session(tcp::socket(io_context), buffer_size)
{
}

session::session(tcp::socket socket, size_t buffer_size)
    : m_socket(std::move(socket)),
      m_buffer(buffer_size)
{
}

void session::connect(const tcp::endpoint& server)
{
    m_socket.async_connect(server, [this](const std::error_code& error) {
        if(error)
        {
           on_error(error);
        }
    });
}

void session::write(const char* bytes, size_t count)
{
    if(count <= buffer_size())
    {
        const bool idle = m_outgoing.empty();
        m_outgoing.push({bytes, bytes + count});
    
        if (idle)
        {
            async_write();
        }
    }
    else
    {
        // Avoid overflow
        on_error(std::make_error_code(std::errc::value_too_large));
    }
}

void session::async_write()
{
    if(!m_outgoing.empty())
    {
        std::vector<char>& outgoing = m_outgoing.front();
        const size_t count = outgoing.size();
        
        auto handler = [this, count](const std::error_code& error, size_t real_count) {
            m_outgoing.pop();
            
            if(error)
            {
                on_error(error);
            }
            else
            {
                assert(count == real_count);
                async_write();
            }
        };
        
        auto buffer = asio::buffer(outgoing.data(), outgoing.size());
        asio::async_write(m_socket, buffer, handler);
    }
}

void session::read(size_t count)
{
    if(count <= buffer_size())
    {
        auto handler = [this, count](const std::error_code& error,
                                     size_t real_count) {
            if(error)
            {
                on_error(error);
            }
            else
            {
                assert(count == real_count);
                on_read(count);
            }
        };
    
        auto buffer = asio::buffer(m_buffer.data(), count);
        asio::async_read(m_socket, buffer, handler); // read exactly `count` bytes
    }
    else
    {
        on_error(std::make_error_code(std::errc::value_too_large));
    }
}

void session::set_on_error(on_error_handler handler)
{
    m_on_error_handler = std::move(handler);
}

void session::set_on_read(on_read_handler handler)
{
    m_on_read_handler = std::move(handler);
}

void session::on_error(const std::error_code& error)
{
    if(m_on_error_handler)
    {
        m_on_error_handler(*this, error);
    }
}

void session::on_read(size_t count)
{
    if(m_on_read_handler)
    {
        m_on_read_handler(*this, m_buffer.data(), count);
    }
}

tcp::endpoint session::remote_endpoint() const
{
    return m_socket.remote_endpoint();
}

tcp::endpoint session::local_endpoint() const
{
    return m_socket.local_endpoint();
}

}
