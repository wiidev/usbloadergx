/* domparser.h
 * libxml++ and this file are copyright (C) 2000 by Ari Johnson, and
 * are covered by the GNU Lesser General Public License, which should be
 * included with libxml++ as the file COPYING.
 */

#ifndef __LIBXMLPP_PARSERS_DOMPARSER_H
#define __LIBXMLPP_PARSERS_DOMPARSER_H

#include <libxml++/parsers/parser.h>
#include <libxml++/dtd.h>
#include <libxml++/document.h>

namespace xmlpp {

/** XML DOM parser.
 *
 */
class DomParser : public Parser
{
public:
  DomParser();

  /** Instantiate the parser and parse a document immediately.
   * @throw exception
   * @param filename The path to the file.
   * @param validate Whether the parser should validate the XML.             
   */  
  explicit DomParser(const std::string& filename, bool validate = false);
  virtual ~DomParser();

  /** Parse an XML document from a file.
   * @throw exception
   * @param filename The path to the file.
   */                                                                                                                                                                                               
  virtual void parse_file(const std::string& filename);

  /** Parse an XML document from a string.
   * @throw exception  
   * @param contents The XML document as a string.
   */ 
  virtual void parse_memory(const std::string& contents);
  
  /** Parse an XML document from raw memory.
   * @throw exception
   * @param contents The XML document as an array of bytes.
   * @param bytes_count The number of bytes in the @a contents array.
   */ 
  void parse_memory_raw(const unsigned char* contents, size_type bytes_count);

  /** Parse an XML document from a stream.
   * @throw exception
   * @param in The stream.
   */ 
  virtual void parse_stream(std::istream& in);

  /** Test whether a document has been parsed.
  */
  operator bool() const;
  
  Document* get_document();
  const Document* get_document() const;
  
protected:
  virtual void parse_context();

  virtual void release_underlying();
  
  Document* doc_;
};




} // namespace xmlpp

#endif //__LIBXMLPP_PARSERS_DOMPARSER_H



