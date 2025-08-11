/* parser.h
 * libxml++ and this file are copyright (C) 2000 by Ari Johnson, and
 * are covered by the GNU Lesser General Public License, which should be
 * included with libxml++ as the file COPYING.
 */

#ifndef __LIBXMLPP_PARSER_H
#define __LIBXMLPP_PARSER_H

#ifdef _MSC_VER //Ignore warnings about the Visual C++ Bug, where we can not do anything
#pragma warning (disable : 4786)
#endif

#include <libxml++/nodes/element.h>
#include <libxml++/exceptions/validity_error.h>
#include <libxml++/exceptions/internal_error.h>

#include <istream>

#ifdef WIN32 //TODO: Why do we do this? murrayc.
#define vsnprintf _vsnprintf
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
extern "C" {
  struct _xmlParserCtxt;
}
#endif //DOXYGEN_SHOULD_SKIP_THIS

namespace xmlpp {

/** XML parser.
 *
 */
class Parser : NonCopyable
{
public:
  Parser();
  virtual ~Parser();

  typedef unsigned int size_type;

  /** By default, the parser will not validate the XML file.
   * @param val Whether the document should be validated.
   */
  virtual void set_validate(bool val = true);

  /** See set_validate()
   * @returns Whether the parser will validate the XML file.
   */
  virtual bool get_validate() const;

  /** Set whether the parser will automatically substitute entity references with the text of the entities' definitions.
   * For instance, this affects the text returned by ContentNode::get_content().
   * By default, the parser will not substitute entities, so that you do not lose the entity reference information.
   * @param val Whether entities will be substitued.
   */
  virtual void set_substitute_entities(bool val = true);

  /** See set_substitute_entities().
   * @returns Whether entities will be substituted during parsing.
   */
  virtual bool get_substitute_entities() const;
  
  /** Parse an XML document from a file.
   * @throw exception
   * @param filename The path to the file.
   */
  virtual void parse_file(const std::string& filename) = 0;

  //TODO: In a future ABI-break, add a virtual void parse_memory_raw(const unsigned char* contents, size_type bytes_count);
  
  /** Parse an XML document from a string.
   * @throw exception
   * @param contents The XML document as a string.
   */
  virtual void parse_memory(const std::string& contents) = 0;

  /** Parse an XML document from a stream.
   * @throw exception
   * @param in The stream.
   */
  virtual void parse_stream(std::istream& in) = 0;

protected:
  virtual void initialize_context();
  virtual void release_underlying();

  virtual void on_validity_error(const std::string& message);
  virtual void on_validity_warning(const std::string& message);

  virtual void handleException(const exception& e);
  virtual void check_for_exception();
  virtual void check_for_validity_messages();
  
  static void callback_validity_error(void* ctx, const char* msg, ...);
  static void callback_validity_warning(void* ctx, const char* msg, ...);
  
  _xmlParserCtxt* context_;
  exception* exception_;
  std::string validate_error_;
  std::string validate_warning_; //Built gradually - used in an exception at the end of parsing.

  bool validate_;
  bool substitute_entities_;
};

} // namespace xmlpp

#endif //__LIBXMLPP_PARSER_H



