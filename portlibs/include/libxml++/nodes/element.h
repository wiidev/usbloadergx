/* element.h
 * libxml++ and this file are copyright (C) 2000 by Ari Johnson, and
 * are covered by the GNU Lesser General Public License, which should be
 * included with libxml++ as the file COPYING.
 */

#ifndef __LIBXMLPP_NODES_ELEMENT_H
#define __LIBXMLPP_NODES_ELEMENT_H

#include <libxml++/nodes/node.h>
#include <libxml++/attribute.h>
#include <libxml++/nodes/commentnode.h>

namespace xmlpp
{

/** Element nodes have attributes as well as child nodes. This will be instantiated by the parser.
 */
class Element : public Node
{
public:
  explicit Element(_xmlNode* node);
  virtual ~Element();

  typedef std::list<Attribute*> AttributeList;

  /** This adds a namespace declaration to this node which will apply to this node and all children.
   * @param ns_uri The namespace to associate with the prefix, or to use as the default namespace if no prefix is specified.
   * @param ns_prefix The namespace prefix. If no prefix is specified then the namespace URI will be the default namespace.
   */
  void set_namespace_declaration(const std::string& ns_uri, const std::string& ns_prefix = std::string());

  /** Obtain the list of attributes for this element.
   * @returns The list of attributes.
   */
  AttributeList get_attributes();

  /** Obtain the list of attributes for this element.
   * @returns The list of attributes.
   */
  const AttributeList get_attributes() const;
    
  // FIXME: the following only returns explicitely provided
  // attributes, not default ones declared in the dtd.
  // TOOD: Is this still true? murrayc
  Attribute* get_attribute(const std::string& name,
                           const std::string& ns_prefix = std::string()) const;

  /** Get the value of the attribute with this name, and optionally with this namespace.
   * For finer control, you might use get_attribute() and use the methods of the Attribute class.
   * @param name The name of the attribute whose value will be retrieved.
   * @param ns_prefix Namespace prefix. 
   * @return The text value of the attribute, or an empty string if no such attribute was found.
   *
   * @newin2p20
   */
  std::string get_attribute_value(const std::string& name, 
                                    const std::string& ns_prefix = std::string()) const;

  /** Set the value of the attribute with this name, and optionally with this namespace.
   * A matching attribute will be added if no matching attribute already exists.
   * For finer control, you might want to use get_attribute() and use the methods of the Attribute class.
   * @param name The name of the attribute whose value will change.
   * @param value The new value for the attribute
   * @param ns_prefix Namespace prefix. If the prefix has not been declared then this method will throw an exception.
   * @return The attribute that was changed, or 0 is no suitable Attribute was found.
   */
  Attribute* set_attribute(const std::string& name, const std::string& value,
                           const std::string& ns_prefix = std::string());

  /** Remove the attribute with this name, and optionally with this namespace.
   * @param name The name of the attribute to be removed
   * @param ns_prefix Namespace prefix. If specified, the attribute will be removed only if the attribute has this namespace.
   */
  void remove_attribute(const std::string& name,
                        const std::string& ns_prefix = std::string());
  
  
  /** Get the first child text content node.
   * This is a convenience method, meant as an alternative to iterating over all the child nodes to find the first suitable node then and getting the text directly.
   * @returns The first text node, if any.
   */
  TextNode* get_child_text();

   /** Get the first child text content node.
   * This is a convenience method, meant as an alternative to iterating over all the child nodes to find the first suitable node then and getting the text directly.
   * @returns The first text node, if any.
   */
  const TextNode* get_child_text() const;

  /** Append a new text node.
   * @param content The text. This should be unescaped - see ContentNode::set_content().
   * @returns The new text node.
   */
  TextNode* add_child_text(const std::string& content = std::string());

  /** Add a new text node after the specified existing child node.
   *
   * @newin2p24
   *
   * @param previous_sibling An existing child node.
   * @param content The text. This should be unescaped - see ContentNode::set_content().
   * @returns The new text node.
   */
  TextNode* add_child_text(xmlpp::Node* previous_sibling, const std::string& content = std::string());

  /** Add a new text node before the specified existing child node.
   *
   * @newin2p24
   *
   * @param next_sibling An existing child node.
   * @param content The text. This should be unescaped - see ContentNode::set_content().
   * @returns The new text node.
   */
  TextNode* add_child_text_before(xmlpp::Node* next_sibling, const std::string& content = std::string());

  /** Set the text of the first text node, adding one if necessary.
   * This is a convenience method, meant as an alternative to iterating over all the child nodes to find the first suitable node then and setting the text directly.
   * @param content The text. This should be unescaped - see ContentNode::set_content().
   */
  void set_child_text(const std::string& content);

  /** Discover whether one of the child nodes is a text node.
   * This is a convenience method, meant as an alternative to iterating over all the child nodes and examining them directly.
   * @returns Whether this node has a child text node.
   */
  bool has_child_text() const;

  /** Append a new comment node.
   * @param content The text. This should be unescaped - see ContentNode::set_content().
   * @returns The new comment node.
   */
  CommentNode* add_child_comment(const std::string& content);

protected:
  std::string get_namespace_uri_for_prefix(const std::string& ns_prefix) const;
};

} // namespace xmlpp

#endif //__LIBXMLPP_NODES_ELEMENT_H
