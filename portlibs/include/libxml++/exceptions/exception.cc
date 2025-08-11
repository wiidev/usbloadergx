#include "exception.h"

namespace xmlpp {
  
exception::exception(const std::string& message)
: message_(message)
{
}

exception::~exception() throw()
{}

const char* exception::what() const throw()
{
  return message_.c_str();
}

void exception::Raise() const
{
  #ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
  throw *this;
  #endif //LIBXMLCPP_EXCEPTIONS_ENABLED
}

exception * exception::Clone() const
{
  return new exception(*this);
}

} //namespace xmlpp

