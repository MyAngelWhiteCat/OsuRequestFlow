#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include "domain.h"
#include "message.h"

#include <fstream>

namespace irc {

    namespace handler {

        using namespace std::literals;

        class MessageHandler {
        public:
            void operator()(const std::vector<domain::Message>& messages);

        };

    }

}