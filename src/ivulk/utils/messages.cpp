#include <ivulk/utils/messages.hpp>

#include <sstream>

namespace ivulk::utils {

    std::string makeMessageBase(const char* tag, char marker, std::string type, std::string description)
    {
        std::stringstream ss;
        ss << marker << marker << marker << ' ';
        ss << tag;
        if (!type.empty())
        {
            ss << "[";
            ss << type;
            ss << "]";
        }
        ss << " -- " << description;
        return ss.str();
    }

    std::string makeErrorMessage(std::string type, std::string description)
    {
        return makeMessageBase("ERROR", '!', type, description);
    }
    std::string makeWarningMessage(std::string type, std::string description)
    {
        return makeMessageBase("WARNING", '!', type, description);
    }
    std::string makeInfoMessage(std::string type, std::string description)
    {
        return makeMessageBase("INFO", '~', type, description);
    }
    std::string makeSuccessMessage(std::string type, std::string description)
    {
        return makeMessageBase("SUCCESS", '+', type, description);
    }
} // namespace ivulk::utils
