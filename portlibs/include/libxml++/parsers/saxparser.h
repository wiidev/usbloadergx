/* saxparser.h
 * libxml++ and this file are copyright (C) 2000 by Ari Johnson, and
 * are covered by the GNU Lesser General Public License, which should be
 * included with libxml++ as the file COPYING.
 */

#ifndef __LIBXMLPP_PARSERS_SAXPARSER_H
#define __LIBXMLPP_PARSERS_SAXPARSER_H

#include <libxml++/parsers/parser.h>

#include <list>
#include <deque>
#include <memory>
#include "libxml++/document.h"

#ifndef DOXYGEN_SHOULD_SKIP_THIS
extern "C" {
  struct _xmlSAXHandler;
  struct _xmlEntity;
}
#endif //DOXYGEN_SHOULD_SKIP_THIS

namespace xmlpp {

/** SAX XML parser.
 * Derive your own class and override the on_*() methods.
 */
class SaxParser : public Parser
{
public:
  /**
   * Simple structure used in the start_element callback, in which
   * the attributes are a list of name/value pairs.
   */
  struct Attribute
  {
    std::string name;
    std::string value;

    Attribute(std::string const & name, std::string const & value)
      : name(name), value(value)
      {
      }
  };

  typedef std::deque< Attribute > AttributeList;

  /** This functor is a helper to find an attribute by name in an
   * AttributeList using the standard algorithm std::find_if
   * 
   * Example:@n
   * <code>
   *   std::string name = "foo";@n
   *   AttributeList::const_iterator attribute = std::find_if(attributes.begin(), attributes.end(), AttributeHasName(name));
   * </code>
   */
  struct AttributeHasName
  {
    std::string const & name;

    AttributeHasName(std::string const & name)
      : name(name)
      {
      }

    bool operator()(Attribute const & attribute)
    {
      return attribute.name == name;
    }
  };
  
  /** 
   * @param use_get_entity Set this to true if you will override on_get_entity().
   * In theory, if you do not override on_get_entity() the parser should behave exactly the same
   * whether you use true or false here. But the default implementation of on_get_entity(), needed
   * if you override on_get_entity() might not have the same behaviour as the underlying default
   * behaviour of libxml, so the libxml implementation is the default here.
   */
  SaxParser(bool use_get_entity = false);
  virtual ~SaxParser();

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
  
  /** Parse a chunk of data.
   *
   * This lets you pass a document in small chunks, e.g. from a network
   * connection. The on_* virtual functions are called each time the chunks
   * provide enough information to advance the parser.
   *
   * The first call to parse_chunk will setup the parser. When the last chunk
   * has been parsed, call finish_chunk_parsing() to finish the parse.
   *
   * @throw exception
   * @param chunk The next piece of the XML document.
   */
  virtual void parse_chunk(const std::string& chunk);

  /** Parse a chunk of data.
   *
   * @newin2p24
   *
   * This lets you pass a document in small chunks, e.g. from a network
   * connection. The on_* virtual functions are called each time the chunks
   * provide enough information to advance the parser.
   *
   * The first call to parse_chunk will setup the parser. When the last chunk
   * has been parsed, call finish_chunk_parsing() to finish the parse.
   *
   * @throw exception
   * @param contents The next piece of the XML document as an array of bytes.
   * @param bytes_count The number of bytes in the @a contents array.
   */
  void parse_chunk_raw(const unsigned char* contents, size_type bytes_count);

  /** Finish a chunk-wise parse.
   *
   * Call this after the last call to parse_chunk(). Don't use this function with
   * the other parsing methods.
   */
  virtual void finish_chunk_parsing();

protected:
        
  virtual void on_start_document();
  virtual void on_end_document();
  virtual void on_start_element(const std::string& name, const AttributeList& attributes);
  virtual void on_end_element(const std::string& name);
  virtual void on_characters(const std::string& characters);
  virtual void on_comment(const std::string& text);
  virtual void on_warning(const std::string& text);
  virtual void on_error(const std::string& text);
  virtual void on_fatal_error(const std::string& text);
  virtual void on_cdata_block(const std::string& text);

  /** Override this to receive information about the document's DTD and any entity declarations.
   */
  virtual void on_internal_subset(const std::string& name, const std::string& publicId, const std::string& systemId);

  /** Override this method to resolve entities references in your derived parser, instead of using the default entity resolution,
   * or to be informed when entity references are encountered.
   *
   * If you override this function then you must also specify true for use_get_entity constructor parameter.
   * You will probably need to override on_entity_declaration() as well so that you can use that information when
   * resolving the entity reference.
   *
   * This is known to be difficult, because it requires both an understanding of the W3C specifications and knowledge of the
   * libxml internals. Entity resolution is easier with the DomParser.
   *
   * Call this method in this base class for default processing. For instance, if you just want to know about the existence of
   * an entity reference, without affecting the normal substitution, just override and call the base class.
   *
   * Unlike the DomParser, the SaxParser will also tell you about entity references for the 5 predefined entities.
   *
   * @param name The entity reference name.
   * @returns The resolved xmlEntity for the entity reference. You must include libxml/parser.h in order to use this C struct.
   * This instance will not be freed by the caller.
   */
  virtual _xmlEntity* on_get_entity(const std::string& name);

  /** Override this to receive information about every entity declaration.
   * If you override this function, and you want normal entity substitution to work, then you must call the base class in your override.
   *
   * This would be useful when overriding on_get_entity().
   */
  virtual void on_entity_declaration(const std::string& name, XmlEntityType type, const std::string& publicId, const std::string& systemId, const std::string& content);

  virtual void release_underlying();
  
private:
  virtual void parse();
  
  std::auto_ptr<_xmlSAXHandler> sax_handler_;

  // A separate xmlpp::Document that is just used for entity resolution,
  // and never seen in the API:
  xmlpp::Document entity_resolver_doc_;

  friend struct SaxParserCallback;
};




} // namespace xmlpp

#endif //__LIBXMLPP_PARSERS_SAXPARSER_H



