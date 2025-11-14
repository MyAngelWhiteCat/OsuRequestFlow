#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "file_manager.h"

namespace downloader {



    class Downloader : public std::enable_shared_from_this<Downloader> {

    public:
        Downloader()
            
        {
            
        }

        void Download(std::string_view host, std::string_view port, bool secured = true) {

        }

    private:
        
    };

}