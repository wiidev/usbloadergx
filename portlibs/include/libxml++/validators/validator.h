/* validator.h
 * libxml++ and this file are copyright (C) 2000 by Ari Johnson,
 * (C) 2002-2004 by the libxml dev team and
 * are covered by the GNU Lesser General Public License, which should be
 * included with libxml++ as the file COPYING.
 */

#ifndef __LIBXMLPP_VALIDATOR_H
#define __LIBXMLPP_VALIDATOR_H

#ifdef _MSC_VER //Ignore warnings about the Visual C++ Bug, where we can not do anything
#pragma warning (disable : 4786)
#endif

#include <libxml++/nodes/element.h>
#include <libxml++/exceptions/validity_error.h>
#include <libxml++/exceptions/internal_error.h>

#ifdef WIN32 //TODO: Why do we do this? murrayc.
#define vsnprintf _vsnprintf
#endif

extern "C" {
  struct _xmlValidCtxt;
}

namespace xmlpp {

/** XML parser.
 *
 */
class Validator : NonCopyable
{
public:
  Validator();
  virtual ~Validator();

protected:
  virtual void initialize_valid();
  virtual void release_underlying();

  virtual void on_validity_error(const std::string& message);
  virtual void on_validity_warning(const std::string& message);

  virtual void handleException(const exception& e);
  virtual void check_for_exception();
  virtual void check_for_validity_messages();

  static void callback_validity_error(void* ctx, const char* msg, ...);
  static void callback_validity_warning(void* ctx, const char* msg, ...);

  _xmlValidCtxt* valid_;
  exception* exception_;
  std::string validate_error_;
  std::string validate_warning_; //Built gradually - used in an exception at the end of parsing.
};

} // namespace xmlpp

#endif //__LIBXMLPP_VALIDATOR_H

