#include "common.h"
#include "sized_session.h"

class Server
{
public:
    Server(asio::io_context& io_context, unsigned short port)
        : m_io_context(io_context),
          m_acceptor(m_io_context, tcp::endpoint(tcp::v4(), port))
    {
    }
    
    /**
     * Call once to initiate the process.
     * It will wait a new client, and then call itself again
     * to wait again a new client.
     */
    void async_accept()
    {
        // As this is a smart ptr,
        // it will not be destroyed at the end of async_accept()
        // because it is captured in the lambda
        auto socket = std::make_shared<tcp::socket>(m_io_context);
        
        // async_ functions returns immediately
        // it will only be run in io_context.run() or io_context.poll()
        // Capture by VALUE so the socket is not destroyed at the end of async_accept()
        m_acceptor.async_accept(
            *socket,
            [this, socket](
                const std::system_error& error) {
            
            if(!error.code())
            {
                std::cout << "client accepted!" << std::endl;
    
                // dereference twice: first iterator, second smart ptr
                auto session = std::make_shared<io::sized_session>(std::move(*socket));
                m_clients.emplace(session);
                
                session->set_on_read([this](io::sized_session& session, const char* bytes, size_t count) {
                    std::string message(bytes, bytes + count);
                    message = join("[", session.remote_endpoint(), "] ", message);
                    
                    std::cout << message.c_str() << std::endl;
                    
                    // Write back to all clients (except to the one who sent the message)
                    for(auto& client : m_clients)
                    {
                        client->write(message.data(), message.size());
                    }
    
                    // Read next message async.
                    session.read();
                });
                
                session->set_on_error([](io::sized_session&, const std::error_code& error) {
                    if(error == asio::error::eof)
                    {
                        std::cout << "client disconnected!" << std::endl;
                    }
                    else
                    {
                        std::cout << "error: " << error.message() << std::endl;
                    }
                });
    
                // Read first message async.
                session->read();
            }
            else
            {
                std::cerr << "acceptor error: " << error.what() << std::endl;
            }
    
            async_accept(); // Listen new client
    
            // At the end of the scope shared_ptr<socket> is destroyed,
            // But it is empty since we moved it into the Client
            // (if there is no error. if there is one, it's just destroyed)
        });
    }
    
private:
    asio::io_context &m_io_context;
    tcp::acceptor m_acceptor;
    std::unordered_set<std::shared_ptr<io::sized_session>> m_clients;
};

int main()
{
    asio::io_context io_context;
    Server handler(io_context, SERVER_LISTENING_PORT);
    
    handler.async_accept();
    
    // Infinite loop since we never stop it
    // and a TCP acceptor will run forever.
    // But it doesn't make a Clang-tidy warning, which is also nice
    while(!io_context.stopped())
    {
        using namespace std::chrono_literals;
        
        std::this_thread::sleep_for(100ms); // Simulates loop payload
        
        io_context.poll();
    }
    
    return 0;
}