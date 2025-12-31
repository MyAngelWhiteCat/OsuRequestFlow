#include "http_server.h"

#include <boost/asio/dispatch.hpp>

namespace http_server {

    void SessionBase::Run() {
        net::dispatch(stream_.get_executor(), beast::bind_front_handler(&SessionBase::Read, GetSharedThis()));
    }

    void SessionBase::OnWrite(bool close, beast::error_code ec, [[maybe_unused]] std::size_t bytes_writen) {
        if (ec) {
            return logging::ReportError(ec, "write");
        }

        if (close) {
            return Close();
        }

        Read();
    }

    void SessionBase::Read() {
        using namespace std::literals;
        request_ = {};
        //stream_.expires_after(30s);
        http::async_read(stream_, buffer_, request_, beast::bind_front_handler(&SessionBase::OnRead, GetSharedThis()));
    }

    void SessionBase::OnRead(beast::error_code ec, [[maybe_unused]] std::size_t bytes_read) {
        if (ec == http::error::end_of_stream) {
            return Close();
        }

        if (ec) {
            return logging::ReportError(ec, "read");
        }

        HandleRequest(std::move(request_));
    }

    void SessionBase::Close() {
        beast::error_code ec;
        stream_.socket().shutdown(net::socket_base::shutdown_send, ec);
        if (ec) {
            logging::ReportError(ec, "Shutdown socket");
        }
    }

}