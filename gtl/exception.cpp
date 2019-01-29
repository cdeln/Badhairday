#include "exception.hpp"

namespace gtl {

    Exception::Exception(const std::string & message) {
        message_ = message;
    }
    const char* Exception::what() const throw() {
        return message_.c_str();
    }

}
