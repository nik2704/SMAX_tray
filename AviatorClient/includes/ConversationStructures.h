
#pragma once

#include <string>
#include <vector>


namespace smax {
 
struct ConversationParameters {
    std::string id;
    std::string tag;
    std::string name;
    std::string client;
};

struct ContextDoc{
    std::string id;
    std::string type;
    std::string label;
    std::string url;
};

struct LastContext {
    std::string text;
    std::string searchedContext;
    std::vector<ContextDoc> referencedDocs;
};

} // namespace smax