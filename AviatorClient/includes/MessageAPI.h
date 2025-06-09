#pragma once

#include <string>

struct MessageAPI {
    std::string id;
    std::string topic_id;
    std::string role;
    std::string content;
};