/* document.h
 * this file is part of libxml++
 *
 * parts of the code copyright (C) 2003 by Stefan Seefeld
 * others copyright (C) 2003 by libxml++ developer's team
 *
 * this file is covered by the GNU Lesser General Public License,
 * which should be included with libxml++ as the file COPYING.
 */

#ifndef __LIBXMLPP_DOCUMENT_H
#define __LIBXMLPP_DOCUMENT_H

#include <libxml++/exceptions/parse_error.h>
#include <libxml++/exceptions/internal_error.h>
#include <libxml++/nodes/element.h>
#include <libxml++/dtd.h>

#include <ostream>

#ifndef DOXYGEN_SHOULD_SKIP_THIS
extern "C" {
  struct _xmlDoc;
  struct _xmlEntity;
}
#endif //DOXYGEN_SHOULD_SKIP_THIS

namespace xmlpp 
{

typedef enum {
    XML_INTERNAL_GENERAL_ENTITY = 1,
    XML_EXTERNAL_GENERAL_PARSED_ENTITY = 2,
    XML_EXTERNAL_GENERAL_UNPARSED_ENTITY = 3,
    XML_INTERNAL_PARAMETER_ENTITY = 4,
    XML_EXTERNAL_PARAMETER_ENTITY = 5,
    XML_INTERNAL_PREDEFINED_ENTITY = 6
} XmlEntityType;

class Document;

//TODO: Make Document inherit from Node, when we can break ABI one day?
//
//libxml might intend xmlDox to derive (theoretically) from xmlNode.
//This is suggested because the XmlNodeSet returned by xmlXPathEval (see the Node::find() implementation) can contain either xmlNode or xmlDocument elements.
/**
 * Represents an XML document in the DOM model.
 */
class Document : NonCopyable
{
  //Ensure that libxml is properly initialised:
  class Init
  {
  public:
    Init();
    virtual ~Init();
  };

  friend class DomParser;
  friend class SaxParser;

public:
  explicit Document(const std::string& version = "1.0");
  virtual ~Document();
  
  /** @return The encoding used in the source from which the document has been loaded.
    */
  std::string get_encoding() const;
  
  Dtd* get_internal_subset() const;
  
  void set_internal_subset(const std::string& name,
                           const std::string& external_id,
                           const std::string& system_id);

  /** Return the root node.
   * This function does _not_ create a default root node if it doesn't exist.
   * @return A pointer to the root node if it exists, 0 otherwise.
   */
  Element* get_root_node() const;

  /** Creates the root node.
   * @param name The node's name.
   * @param ns_uri The namespace URI. A namspace declaration will be added to this node, because it could not have
     been declared before.
   * @param ns_prefix The namespace prefix to associate with the namespace. If no namespace prefix is specified then
     the namespace URI will be the default namespace.
   * @return A pointer to the new root node
   */
  Element* create_root_node(const std::string& name,
                            const std::string& ns_uri = std::string(),
                            const std::string& ns_prefix = std::string() );

  /** Creates a root node by importing the node from another document, without affecting the source node.
   * @param node The node to copy and insert as the root node of the document
   * @param recursive Whether to import the child nodes also. Defaults to true.
   * @return A pointer to the new root node
   */
  Element* create_root_node_by_import(const Node* node,
				      bool recursive = true);

  /** Append a new comment node.
   * @param content The text. This should be unescaped - see ContentNode::set_content().
   * @returns The new comment node.
   */
  CommentNode* add_comment(const std::string& content);

  //TODO: Use std::string for filenames.
  /** Write the document to a file.
   * @param filename
   * @param encoding If not provided, UTF-8 is used
   */
  void write_to_file(const std::string& filename, const std::string& encoding = std::string());

  /** Write the document to a file.
   * The output is formatted by inserting whitespaces, which is easier to read for a human,
   * but may insert unwanted significant whitespaces. Use with care !
   * @param filename
   * @param encoding If not provided, UTF-8 is used
   */
  void write_to_file_formatted(const std::string& filename, const std::string& encoding = std::string());

  /** Write the document to the memory.
   * @param encoding If not provided, UTF-8 is used
   */
  std::string write_to_string(const std::string& encoding = std::string());

  /** Write the document to the memory.
   * The output is formatted by inserting whitespaces, which is easier to read for a human,
   * but may insert unwanted significant whitespaces. Use with care !
   * @param encoding If not provided, UTF-8 is used
   * @return The written document.
   */
  std::string write_to_string_formatted(const std::string& encoding = std::string());

  /** Write the document to a std::ostream.
   * @param output A reference to the stream in which the document will be written
   * @param encoding If not provided, UTF-8 is used
   * @warning This method is much less efficient than write_to_string if you want to dump the
   * document to a buffer or the standard output. Writing to a fstream is almost as fast as write_to_file
   */
  void write_to_stream(std::ostream& output, const std::string& encoding = std::string());

  /** Write the document to a std::ostream.
   * The output is formatted by inserting whitespaces, which is easier to read for a human,
   * but may insert unwanted significant whitespaces. Use with care !
   * @param output A reference to the stream in which the document will be written
   * @param encoding If not provided, UTF-8 is used
   * @warning See write_to_stream
   */
  void write_to_stream_formatted(std::ostream & output, const std::string& encoding = std::string());

  /** Add an Entity declaration to the document.
   * @param name The name of the entity that will be used in an entity reference.
   * @param type The type of entity.
   * @param publicId The public ID of the subset.
   * @param systemId The system ID of the subset.
   * @param content The value of the Entity. In entity reference substitutions, this
   * is the replacement value.
   */
  virtual void set_entity_declaration(const std::string& name, XmlEntityType type,
                                      const std::string& publicId, const std::string& systemId,
                                      const std::string& content);

  ///Access the underlying libxml implementation.
  _xmlDoc* cobj();

  ///Access the underlying libxml implementation.
  const _xmlDoc* cobj() const;

protected:
  /** Retrieve an Entity.
   * The entity can be from an external subset or internally declared.
   * @param name Then name of the entity to get.
   * @returns A pointer to the libxml2 entity structure.
   */
  _xmlEntity* get_entity(const std::string& name);

private:
  virtual void do_write_to_file(const std::string& filename, const std::string& encoding, bool format);
  virtual std::string do_write_to_string(const std::string& encoding, bool format);
  virtual void do_write_to_stream(std::ostream& output, const std::string& encoding, bool format);

  static Init init_;

  Document(_xmlDoc* doc);
  _xmlDoc* impl_;
};

} //namespace xmlpp

#endif //__LIBXMLPP_DOCUMENT_H
