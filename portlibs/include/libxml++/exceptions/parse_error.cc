#include "parse_error.h"

namespace xmlpp {

parse_error::parse_error(const std::string& message)
: exception(message)
{
}

parse_error::~parse_error() throw()
{}

void parse_error::Raise() const
{
  #ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
  throw *this;
  #endif //LIBXMLCPP_EXCEPTIONS_ENABLED
}

exception* parse_error::Clone() const
{
  return new parse_error(*this);
}

} //namespace xmlpp

