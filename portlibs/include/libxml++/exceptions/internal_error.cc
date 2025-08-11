#include "internal_error.h"


namespace xmlpp {

internal_error::internal_error(const std::string& message)
: exception(message)
{
}

internal_error::~internal_error() throw()
{}

void internal_error::Raise() const
{
  #ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
  throw *this;
  #endif //LIBXMLCPP_EXCEPTIONS_ENABLED
}

exception * internal_error::Clone() const
{
  return new internal_error(*this);
}

} //namespace xmlpp


