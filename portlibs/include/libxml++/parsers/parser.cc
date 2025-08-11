/* parser.cc
 * libxml++ and this file are copyright (C) 2000 by Ari Johnson, and
 * are covered by the GNU Lesser General Public License, which should be
 * included with libxml++ as the file COPYING.
 */

#include "libxml++/parsers/parser.h"

#include <libxml/parser.h>

#include <cstdarg> //For va_list.
#include <memory> //For auto_ptr.

namespace xmlpp {

Parser::Parser()
: context_(0), exception_(0), validate_(false), substitute_entities_(false) //See doxygen comment on set_substiute_entities().
{

}

Parser::~Parser()
{
  release_underlying();
}

void Parser::set_validate(bool val)
{
  validate_ = val;
}

bool Parser::get_validate() const
{
  return validate_;
}

void Parser::set_substitute_entities(bool val)
{
  substitute_entities_ = val;
}

bool Parser::get_substitute_entities() const
{
  return substitute_entities_;
}

void Parser::initialize_context()
{
  //Disactivate any non-standards-compliant libxml1 features.
  //These are disactivated by default, but if we don't deactivate them for each context
  //then some other code which uses a global function, such as xmlKeepBlanksDefault(),
  // could cause this to use the wrong settings:
  context_->linenumbers = 1; // TRUE - This is the default anyway.

  //Turn on/off validation:
  context_->validate = (validate_ ? 1 : 0);

  //Tell the validity context about the callbacks:
  //(These are only called if validation is on - see above)
  context_->vctxt.error = &callback_validity_error;
  context_->vctxt.warning = &callback_validity_warning;

  //Allow the callback_validity_*() methods to retrieve the C++ instance:
  context_->_private = this;

  //Whether or not we substitute entities:
  context_->replaceEntities = (substitute_entities_ ? 1 : 0);

  //Clear these temporary buffers too:
  validate_error_.erase();
  validate_warning_.erase();
}

void Parser::release_underlying()
{
  if(context_)
  {
    context_->_private = 0; //Not really necessary.
    
    if( context_->myDoc != NULL )
    {
      xmlFreeDoc(context_->myDoc);
    }

    xmlFreeParserCtxt(context_);
    context_ = 0;
  }
}

void Parser::on_validity_error(const std::string& message)
{
  //Throw an exception later when the whole message has been received:
  validate_error_ += message;
}

void Parser::on_validity_warning(const std::string& message)
{
  //Throw an exception later when the whole message has been received:
  validate_warning_ += message;
}

void Parser::check_for_validity_messages()
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
  
void Parser::callback_validity_error(void* context_, const char* msg, ...)
{
  //See xmlHTMLValidityError() in xmllint.c in libxml for more about this:
  
  xmlParserCtxtPtr context = (xmlParserCtxtPtr)context_;
  if(context)
  {
    Parser* parser = static_cast<Parser*>(context->_private);
    if(parser)
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
      #endif
        parser->on_validity_error(std::string(buff));
      #ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
      }
      catch(const exception& e)
      {
        parser->handleException(e);
      }
      #endif
    }
  }
  
}

void Parser::callback_validity_warning(void* context_, const char* msg, ...)
{
  //See xmlHTMLValidityError() in xmllint.c in libxml for more about this:
  
  xmlParserCtxtPtr context = (xmlParserCtxtPtr)context_;
  if(context)
  {
    Parser* parser = static_cast<Parser*>(context->_private);
    if(parser)
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
      #endif
        parser->on_validity_warning(std::string(buff));
      #ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
      }
      catch(const exception& e)
      {
        parser->handleException(e);
      }
      #endif
    }
  }
}

void Parser::handleException(const exception& e)
{
  exception_ = e.Clone();

  if(context_)
    xmlStopParser(context_);

  //release_underlying();
}

void Parser::check_for_exception()
{
  check_for_validity_messages();
  
  if(exception_)
  {
    std::auto_ptr<exception> tmp ( exception_ );
    exception_ = 0;
    tmp->Raise();
  }
}

} // namespace xmlpp


