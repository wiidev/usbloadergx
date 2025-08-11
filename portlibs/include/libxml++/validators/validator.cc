/* xml++.cc
 * libxml++ and this file are copyright (C) 2000 by Ari Johnson
 * (C) 2002-2004 by the libxml dev team and
 * are covered by the GNU Lesser General Public License, which should be
 * included with libxml++ as the file COPYING.
 */

#include "libxml++/validators/validator.h"

#include <libxml/parser.h>

#include <cstdarg> //For va_list.

namespace xmlpp {

Validator::Validator()
: valid_(0), exception_(0)
{
}

Validator::~Validator()
{
  release_underlying();
}

void Validator::initialize_valid()
{
  //Tell the validity valid about the callbacks:
  //(These are only called if validation is on - see above)
  valid_->error = &callback_validity_error;
  valid_->warning = &callback_validity_warning;

  //Allow the callback_validity_*() methods to retrieve the C++ instance:
  valid_->userData = this;

  //Clear these temporary buffers too:
  validate_error_.erase();
  validate_warning_.erase();
}

void Validator::release_underlying()
{
  if(valid_)
  {
    valid_->userData = 0; //Not really necessary.

    xmlFreeValidCtxt(valid_);
    valid_ = 0;
  }
}

void Validator::on_validity_error(const std::string& message)
{
  //Throw an exception later when the whole message has been received:
  validate_error_ += message;
}

void Validator::on_validity_warning(const std::string& message)
{
  //Throw an exception later when the whole message has been received:
  validate_warning_ += message;
}

void Validator::check_for_validity_messages()
{
  if(!validate_error_.empty())
  {
    if(!exception_)
      exception_ = new validity_error("Validity error:\n" + validate_error_);

    validate_error_.erase();
  }

  if(!validate_warning_.empty())
  {
    if(!exception_)
      exception_ = new validity_error("Validity warning:\n" + validate_warning_);

    validate_warning_.erase();
  }
}

void Validator::callback_validity_error(void* valid_, const char* msg, ...)
{
  Validator* validator = static_cast<Validator*>(valid_);

  if(validator)
  {
    //Convert the ... to a string:
    va_list arg;
    char buff[1024]; //TODO: Larger/Shared

    va_start(arg, msg);
    vsnprintf(buff, sizeof(buff)/sizeof(buff[0]), msg, arg);
    va_end(arg);

    #ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
    try
    {
    #endif //LIBXMLCPP_EXCEPTIONS_ENABLED
      validator->on_validity_error(std::string(buff));
    #ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
    }
    catch(const exception& e)
    {
      validator->handleException(e);
    }
    #endif //LIBXMLCPP_EXCEPTIONS_ENABLED
  }
}

void Validator::callback_validity_warning(void* valid_, const char* msg, ...)
{
  Validator* validator = static_cast<Validator*>(valid_);

  if(validator)
  {
    //Convert the ... to a string:
    va_list arg;
    char buff[1024]; //TODO: Larger/Shared

    va_start(arg, msg);
    vsnprintf(buff, sizeof(buff)/sizeof(buff[0]), msg, arg);
    va_end(arg);

    #ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
    try
    {
    #endif //LIBXMLCPP_EXCEPTIONS_ENABLED
      validator->on_validity_warning(std::string(buff));
    #ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
    }
    catch(const exception& e)
    {
      validator->handleException(e);
    }
    #endif //LIBXMLCPP_EXCEPTIONS_ENABLED
  }
}

void Validator::handleException(const exception& e)
{
  exception_ = e.Clone();

  release_underlying();
}

void Validator::check_for_exception()
{
  check_for_validity_messages();

  if(exception_)
  {
    exception* tmp = exception_;
    exception_ = 0;
    tmp->Raise();
  }
}

} // namespace xmlpp


