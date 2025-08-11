/* element.cc
 * libxml++ and this file are copyright (C) 2000 by Ari Johnson, and
 * are covered by the GNU Lesser General Public License, which should be
 * included with libxml++ as the file COPYING.
 */

#include <libxml++/nodes/element.h>
#include <libxml++/nodes/textnode.h>
#include <libxml++/exceptions/internal_error.h>

#include <libxml/tree.h>

namespace xmlpp
{

Element::Element(xmlNode* node)
: Node(node)
{}

Element::~Element()
{}

Element::AttributeList Element::get_attributes()
{
  AttributeList attributes;
  for(xmlAttr* attr = cobj()->properties; attr; attr = attr->next)
  {
    attributes.push_back(reinterpret_cast<Attribute*>(attr->_private));
  }

  return attributes;
}

const Element::AttributeList Element::get_attributes() const
{
  return const_cast<Element*>(this)->get_attributes();
}

Attribute* Element::get_attribute(const std::string& name,
                                  const std::string& ns_prefix) const
{
  if(ns_prefix.empty())
  {
    xmlAttr* attr = xmlHasProp(const_cast<xmlNode*>(cobj()), (const xmlChar*)name.c_str());
    if( attr )
    {
      return reinterpret_cast<Attribute*>(attr->_private);
    }
  }
  else
  {
    std::string ns_uri = get_namespace_uri_for_prefix(ns_prefix);  
    xmlAttr* attr = xmlHasNsProp(const_cast<xmlNode*>(cobj()), (const xmlChar*)name.c_str(),
                                 (const xmlChar*)ns_uri.c_str());
    if( attr )
    {
      return reinterpret_cast<Attribute*>(attr->_private);
    }
  }

  return 0;
}

std::string Element::get_attribute_value(const std::string& name, const std::string& ns_prefix) const
{
  const Attribute* attr = get_attribute(name, ns_prefix);
  return attr ? attr->get_value() : std::string();
}

Attribute* Element::set_attribute(const std::string& name, const std::string& value,
                                  const std::string& ns_prefix)
{
  xmlAttr* attr = 0;

  //Ignore the namespace if none was specified:
  if(ns_prefix.empty())
  {
    attr = xmlSetProp(cobj(), (const xmlChar*)name.c_str(), (const xmlChar*)value.c_str());
  }
  else
  {
    //If the namespace exists, then use it:
    xmlNs* ns = xmlSearchNs(cobj()->doc, cobj(), (const xmlChar*)ns_prefix.c_str());
    if (ns)
    {
      attr = xmlSetNsProp(cobj(), ns, (const xmlChar*)name.c_str(),
                          (const xmlChar*)value.c_str());
    }
    else
    {
      #ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
      throw exception("The namespace prefix (" + ns_prefix + ") has not been declared.");
      #endif //LIBXMLCPP_EXCEPTIONS_ENABLED
    }
  }

  if(attr)
    return reinterpret_cast<Attribute*>(attr->_private);
  else
    return 0;
}

void Element::remove_attribute(const std::string& name, const std::string& ns_prefix)
{
  if (ns_prefix.empty())
    xmlUnsetProp(cobj(), (const xmlChar*)name.c_str());
  else
  {
    xmlNs* ns = xmlSearchNs(cobj()->doc, cobj(), (const xmlChar*)ns_prefix.c_str());
    if (ns)
      xmlUnsetNsProp(cobj(), ns, (const xmlChar*)name.c_str());
  }
}

const TextNode* Element::get_child_text() const
{
  // FIXME: return only the first content node
  for(xmlNode* child = cobj()->children; child; child = child->next)
     if(child->type == XML_TEXT_NODE)
        return static_cast<TextNode*>(child->_private);

  return 0;
}

TextNode* Element::get_child_text()
{
  // TODO: This only returns the first content node.
  // What should we do instead? Update the documentation if we change this. murrayc.
  for(xmlNode* child = cobj()->children; child; child = child->next)
     if(child->type == XML_TEXT_NODE)
        return static_cast<TextNode*>(child->_private);

  return 0;
}

void Element::set_child_text(const std::string& content)
{
  TextNode* node = get_child_text();
  if(node)
    node->set_content(content);
  else
    add_child_text(content);
}

TextNode* Element::add_child_text(const std::string& content)
{
  if(cobj()->type == XML_ELEMENT_NODE)
  {
     xmlNode* node = xmlNewText((const xmlChar*)content.c_str());

     // Use the result, because node can be freed when merging text nodes:
     node = xmlAddChild(cobj(), node); 

     return static_cast<TextNode*>(node->_private);
  }
  return 0;
}

TextNode* Element::add_child_text(xmlpp::Node* previous_sibling, const std::string& content)
{
  if(!previous_sibling)
    return 0;

  if(cobj()->type == XML_ELEMENT_NODE)
  {
     xmlNode* node = xmlNewText((const xmlChar*)content.c_str());

     // Use the result, because node can be freed when merging text nodes:
     node = xmlAddNextSibling(previous_sibling->cobj(), node); 

     return static_cast<TextNode*>(node->_private);
  }
  return 0;
}

TextNode* Element::add_child_text_before(xmlpp::Node* next_sibling, const std::string& content)
{
  if(!next_sibling)
    return 0;

  if(cobj()->type == XML_ELEMENT_NODE)
  {
     xmlNode* node = xmlNewText((const xmlChar*)content.c_str());

     // Use the result, because node can be freed when merging text nodes:
     node = xmlAddPrevSibling(next_sibling->cobj(), node); 

     return static_cast<TextNode*>(node->_private);
  }
  return 0;
}

bool Element::has_child_text() const
{
  return get_child_text() != 0;
}

void Element::set_namespace_declaration(const std::string& ns_uri, const std::string& ns_prefix)
{
  //Create a new namespace declaration for this element:
  xmlNewNs(cobj(), (const xmlChar*)(ns_uri.empty() ? 0 : ns_uri.c_str()),
                   (const xmlChar*)(ns_prefix.empty() ? 0 : ns_prefix.c_str()) );
  //We ignore the returned xmlNS*. Hopefully this is owned by the node. murrayc.
}

std::string Element::get_namespace_uri_for_prefix(const std::string& ns_prefix) const
{
  std::string result;
  
  //Find the namespace:
  const xmlNs* ns = xmlSearchNs( cobj()->doc, const_cast<xmlNode*>(cobj()), (xmlChar*)ns_prefix.c_str() );
  if(ns)
  {
    //Get the namespace URI associated with this prefix:
    if(ns && ns->href)
      result = (const char*)ns->href;
  }
  
  return result;
}


CommentNode* Element::add_child_comment(const std::string& content)
{
  xmlNode* node = xmlNewComment((const xmlChar*)content.c_str());
 
  // Use the result, because node can be freed when merging text nodes:
  node = xmlAddChild(cobj(), node);
  return static_cast<CommentNode*>(node->_private);
}





} //namespace xmlpp
