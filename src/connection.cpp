#pragma once

#ifdef WIN32
#include <sdkddkver.h>
#endif


#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ssl/context_base.hpp>
#include <boost/asio/ssl/impl/context.ipp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/ssl/stream_base.hpp>
#include <boost/asio/ssl/verify_mode.hpp>

#include <string>
#include <string_view>
#include <variant>
#include <memory>
#include <stdexcept>

#include "logging.h"
#include "ca_sertificates_loader.h"
#include <openssl/ssl.h>
#include "connection.h"


namespace connection {

    namespace net = boost::asio;
    namespace sys = boost::system;
    namespace ssl = net::ssl;
    using net::ip::tcp;
    using namespace std::literals;

    using Strand = net::strand<net::io_context::executor_type>;


    Connection::Connection(net::io_context& ioc, Strand& write_strand, Strand& read_strand)
        : write_strand_(read_strand)
        , read_strand_(read_strand)
        , socket_(tcp::socket(ioc))
        , ioc_(&ioc)
        , secured_(false)
    {

    }

    Connection::Connection(net::io_context& ioc, ssl::context& ctx, Strand& write_strand, Strand& read_strand)
        : write_strand_(read_strand)
        , read_strand_(read_strand)
        , socket_(ssl::stream<tcp::socket>(ioc, ctx))
        , ioc_(&ioc)
        , secured_(true)
    {

    }

    void Connection::Connect(std::string_view host, std::string_view port) {
        ec_.clear();

        ConnectionVisitor visitor(*this, host, port);
        std::visit(visitor, socket_);
        if (ec_) {
            logging::ReportError(ec_, "Connection");
        }
    }

    void Connection::Disconnect(bool is_need_to_close_socket) {
        ec_.clear();

        DisconnectVisitor visitor(*this, is_need_to_close_socket);
        std::visit(visitor, socket_);
        if (ec_) {
            logging::ReportError(ec_, "Disconnecting");
        }
        LOG_INFO("Disconnected");
    }

    bool Connection::IsReconnectRequired() {
        if (reconnect_required_) {
            reconnect_required_ = false;
            return true;
        }
        return false;
    }

    bool Connection::IsConnected() const {
        return ssl_connected_ || connected_;
    }

    net::io_context* Connection::GetContext() {
        return ioc_;
    }

    bool Connection::IsSecured() const {
        return secured_;
    }

    void Connection::ConnectionVisitor::operator()(tcp::socket& socket) {
        tcp::resolver resolver(socket.get_executor()); // :(
        auto endpoints = resolver.resolve(host_, port_);
        if (connection_.ec_) {
            logging::ReportError(connection_.ec_, "Resolving");
        }
        else {
            LOG_INFO("Resolved "s.append(host_).append(":"s).append(port_));
            for (const auto& ep : endpoints) {
                LOG_INFO(endpoints.begin()->endpoint().address().to_string().append(":"s).append(port_));
            }
        }
        net::connect(socket, endpoints, connection_.ec_);
        if (connection_.ec_) {
            logging::ReportError(connection_.ec_, "Connection"sv);
            return;
        }
        connection_.connected_ = true;
    }

    void Connection::ConnectionVisitor::operator()(ssl::stream<tcp::socket>& socket) {
        tcp::resolver resolver(socket.get_executor()); // :(
        auto endpoints = resolver.resolve(host_, port_, connection_.ec_);

        if (connection_.ec_) {
            logging::ReportError(connection_.ec_, "Resolving");
            throw std::runtime_error("cant resolve: "s.append(host_).append(" ").append(port_));
        }

        LOG_INFO("Resolved "s.append(host_).append(":"s).append(port_));
        for (const auto& ep : endpoints) {
            LOG_INFO(endpoints.begin()->endpoint().address().to_string().append(":"s).append(port_));
        }

        SSL_set_tlsext_host_name(socket.native_handle(), host_.c_str());
        net::connect(socket.lowest_layer(), endpoints, connection_.ec_);
        if (connection_.ec_) {
            logging::ReportError(connection_.ec_, "SSL Connection"sv);
            return;
        }
        else {
            LOG_INFO("CONNECTED");
        }
        socket.lowest_layer().set_option(tcp::no_delay(true));
        socket.handshake(ssl::stream_base::client, connection_.ec_);
        if (connection_.ec_) {
            ERR_print_errors_fp(stderr);

            logging::ReportError(connection_.ec_, "SSL Handshake");
            socket.lowest_layer().close();
            return;
        }

        else {
            LOG_INFO("HANDSHAKE SUCESS");
        }
        connection_.ssl_connected_ = true;
        if (SSL_get_verify_result(socket.native_handle()) != X509_V_OK) {
            LOG_INFO("SSL Certificate verification failed");
        }
        else {
            LOG_INFO("SSL Certificate verified successfully");
        }
    }

    void Connection::DisconnectVisitor::operator()(tcp::socket& socket) {
        socket.shutdown(net::socket_base::shutdown_send, ignor_);
        if (is_need_to_close_socket_) {
            socket.close(connection_.ec_);
        }
        connection_.connected_ = false;
    }

    void Connection::DisconnectVisitor::operator()(ssl::stream<tcp::socket>& socket) {
        socket.shutdown(ignor_);
        if (is_need_to_close_socket_) {
            socket.lowest_layer().close(connection_.ec_);
        }
        connection_.ssl_connected_ = false;
    }

}