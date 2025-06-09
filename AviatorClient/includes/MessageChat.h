// ChatMessage.h - This file ONLY defines ChatMessage
#pragma once

#include <string> // For std::string members


namespace smax {
    
struct MessageChat {
    std::string id;
    std::string text;
    bool from_user;
    std::string timestamp;
};

}