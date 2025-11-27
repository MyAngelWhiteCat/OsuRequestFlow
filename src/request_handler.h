#pragma once

#include "core.h"

#include <memory>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;

namespace gui_http {

    class RequestHandler {
    public:
        template <typename req, typename send>
        void operator()(req&&, send&&) {
            
        }

    private:
        std::unique_ptr<core::Core> core_;

    };

}