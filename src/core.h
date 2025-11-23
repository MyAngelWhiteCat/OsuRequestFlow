#pragma once

#include "irc_client.h"

#include <memory>


namespace core {

    class Core {
    public:
        Core(std::shared_ptr<irc::Client> client)
            : client_(client)
        {
        }

    private:
        std::shared_ptr<irc::Client> client_;
    };

} // namespace core