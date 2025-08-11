#include "validity_error.h"

namespace xmlpp {

validity_error::validity_error(const std::string& message)
: parse_error(message)
{
}

validity_error::~validity_error() throw()
{}

void validity_error::Raise() const
{
  #ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
  throw *this;
  #endif //LIBXMLCPP_EXCEPTIONS_ENABLED
}

exception* validity_error::Clone() const
{
  return new validity_error(*this);
}

} //namespace xmlpp

