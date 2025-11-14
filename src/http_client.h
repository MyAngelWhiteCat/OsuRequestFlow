#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>

#include <boost/beast/core/basic_stream.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/verb.hpp>


#include <memory>
#include <variant>

namespace http {

    namespace net = boost::asio;
    namespace sys = boost::system;
    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace ssl = net::ssl;

    using net::ip::tcp;
    using namespace std::literals;

    using Strand = net::strand<net::io_context::executor_type>;
    using StringResponse = http::response<http::string_body>;
    using StringRequest = http::request<http::string_body>;
    using DynamicResponse = http::response<http::dynamic_body>;

    class Client : public std::enable_shared_from_this<Client> {
    public:

    private:
        std::variant<beast::tcp_stream, ssl::stream<beast::tcp_stream>> stream_;
        beast::flat_buffer buffer_;
        StringResponse response_;

        StringRequest MakeRequest(http::verb method, ) {

        }

        template <typename Handler>
        class ReadVisitor : public std::enable_shared_from_this<ReadVisitor> {
        public:
            
            void operator()(beast::tcp_stream& stream) {
            
            }

            void operator()(ssl::stream<beast::tcp_stream>> stream) {
            
            }

        private:
            Handler&& handler_;
            std::shared_ptr<Client> client_;

            template <typename Stream>
            void Read(Stream& stream) {
                http::async_read(stream, )
            }

        };

        template <typename Handler>
        class WriteVisitor : public std::enable_shared_from_this<WriteVisitor> {
        public:
            
            void operator()(beast::tcp_stream& stream) {
                
            }

            void operator()(ssl::stream<beast::tcp_stream >> stream) {
            
            }
        
        private:
            Handler&& handler_;
            std::shared_ptr<Client> client_;

        };
    };

}