#ifndef __GTL_EXCEPTION__
#define __GTL_EXCEPTION__

#include <exception>
#include <string>

namespace gtl {
    // https://stackoverflow.com/questions/17438863/c-exceptions-with-message
    class Exception : public std::exception {
        private:
            std::string message_;
        public:
            explicit Exception(const std::string & message);
            virtual const char* what() const throw();
    };

}

#endif
