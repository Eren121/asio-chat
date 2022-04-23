#include "sized_session.h"

namespace io
{

sized_session::sized_session(asio::io_context& io_context, size_t buffer_size)
    : sized_session(tcp::socket(io_context), buffer_size)
{
}

sized_session::sized_session(tcp::socket socket, size_t buffer_size)
    : m_session(std::move(socket), buffer_size),
      m_in_header(false)
{
    m_session.set_on_error([this](session&, const std::error_code& code) {
        if(m_on_error_handler)
        {
            m_on_error_handler(*this, code);
        }
    });
    
    m_session.set_on_read([this](session&, const char* bytes, size_t count) {
        if(m_in_header)
        {
            assert(count == sizeof(header_size_type));
            header_size_type header;
            std::copy(bytes, bytes + count, reinterpret_cast<char*>(&header));

            m_in_header = false;
            
            // Finally ask to read the user-data message
            m_session.read(header);
        }
        else
        {
            if(m_on_read_handler)
            {
                m_on_read_handler(*this, bytes, count);
            }
        }
    });
}

void sized_session::write(const char *bytes, size_t count)
{
    header_size_type header = count;
    m_session.write(reinterpret_cast<const char*>(&header), sizeof(header));
    m_session.write(bytes, count);
}

void sized_session::read()
{
    // One on two message is the header (= size of the user-data message)
    
    m_in_header = true;
    m_session.read(sizeof(header_size_type));
}

}