#include "common.h"
#include "sized_session.h"
#include <atomic>
#include <mutex>
#include <algorithm>

// We have to type and print messages at the same time.
// C++ I/O is blocking and we don't use GUI like Qt for simplicity,
// We can still use non-blocking std::cin by using another thread.
// It may overlap in console but that is for the sake of understanding.
class StdinReader
{
public:
    StdinReader()
        : m_thread([this]() { run(); }),
          m_running(true)
    {
    }
    
    ~StdinReader()
    {
        if(m_thread.joinable())
        {
            // A thread may be not joinable if it is not run for example,
            // But here it should always be joinable because we run it in constructor
            m_thread.join();
        }
    }
    
    // Consume all pending messages
    [[nodiscard]] std::queue<std::string> consume()
    {
        std::lock_guard guard(m_mutex);
        return std::move(m_messages);
    }
    
    bool running() const
    {
        return m_running;
    }
    
private:
    // Run in another thread
    void run()
    {
        while(m_running)
        {
            std::string message;
            
            // cin is blocking and there is no portable way to interrupt it
            // so the connection may be closed and the user still writing a message
            if(!(std::getline(std::cin, message)))
            {
                std::cout << "null character sent, stop asking input." << std::endl;
                m_running = false;
            }
            else
            {
                std::lock_guard guard(m_mutex);
                m_messages.push(message);
            }
        }
    }
    
private:
    // Messages that the user type and are not already sent
    // The network is probably very quick and the size will be almost always <= 1
    // unless the user types very quick
    std::queue<std::string> m_messages;
    std::thread m_thread;
    bool m_running;
    
    std::mutex m_mutex; // Make messages thread-safe
};

int main()
{
    asio::io_context io_context;
    StdinReader reader;
    io::sized_session session(io_context);
    
    tcp::endpoint server_address(
        ip::make_address("127.0.0.1"),
        SERVER_LISTENING_PORT
    );
    
    session.connect(server_address);
    session.set_on_error([&](io::sized_session&, const std::error_code& error) {
        if(error == asio::error::eof)
        {
            std::cout << "disconnected!" << std::endl;
            io_context.stop();
        }
        else
        {
            std::cout << "error: " << error.message() << std::endl;
        }
    });
    
    session.set_on_read([&](io::sized_session&, const char* bytes, size_t count) {
        // `bytes` may be not null-terminated
        auto end = std::find(bytes, bytes + count, '\0');
        std::string message(bytes, end);
        
        std::cout << message << std::endl;
        
        // Read next message async.
        session.read();
    });
    
    // Read first message async.
    session.read();
    
    
    // Sometimes the io_context will have nothing to do and stop.
    // We prevent it by adding a guard.
    [[maybe_unused]] auto work_guard = asio::make_work_guard(io_context);
    
    while(!io_context.stopped())
    {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(100ms); // Simulates loop payload
        
        {
            std::queue<std::string> to_send = reader.consume();
            while(!to_send.empty())
            {
                std::string message = to_send.front();
                
                to_send.pop();
                
                std::cout << "send: \"" << message << "\"" << std::endl;
                
                session.write(message.data(), message.size());
            }
        }
        
        io_context.poll();
    }
    
    std::cout << "connection stopped." << std::endl;
    
    return 0;
}