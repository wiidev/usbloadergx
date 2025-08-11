/* node.cc
 * libxml++ and this file are copyright (C) 2000 by Ari Johnson, and
 * are covered by the GNU Lesser General Public License, which should be
 * included with libxml++ as the file COPYING.
 */

#include <libxml++/nodes/element.h>
#include <libxml++/nodes/node.h>
#include <libxml++/exceptions/internal_error.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/tree.h>

#include <iostream>

namespace xmlpp
{

Node::Node(xmlNode* node)
  : impl_(node)
{
   impl_->_private = this;
}

Node::~Node()
{}

const Element* Node::get_parent() const
{
  return cobj()->parent && cobj()->parent->type == XML_ELEMENT_NODE ? 
             static_cast<const Element*>(cobj()->parent->_private) : NULL;
}

Element* Node::get_parent()
{
  return cobj()->parent && cobj()->parent->type == XML_ELEMENT_NODE ? 
            static_cast<Element*>(cobj()->parent->_private) : NULL;
}

const Node* Node::get_next_sibling() const
{
  return const_cast<Node*>(this)->get_next_sibling();
}

Node* Node::get_next_sibling()
{
  return cobj()->next ? 
	        static_cast<Node*>(cobj()->next->_private) : NULL;
}

const Node* Node::get_previous_sibling() const
{
  return const_cast<Node*>(this)->get_previous_sibling();
}

Node* Node::get_previous_sibling()
{
  return cobj()->prev ? 
            static_cast<Node*>(cobj()->prev->_private) : NULL;
}

Node::NodeList Node::get_children(const std::string& name)
{
   xmlNode* child = impl_->children;
   if(!child)
     return NodeList();

   NodeList children;
   do
   {
      if(child->_private)
      {
        if(name.empty() || name == (const char*)child->name)
          children.push_back(reinterpret_cast<Node*>(child->_private));
      }
      else
      {
        //This should not happen:
        //This is for debugging only:
        //if(child->type == XML_ENTITY_DECL)
        //{
        //  xmlEntity* centity = (xmlEntity*)child;
        //  std::cerr << "Node::get_children(): unexpected unwrapped Entity Declaration node name =" << centity->name << std::endl;
        //}
      }
   }
   while((child = child->next));
   
   return children;
}

const Node::NodeList Node::get_children(const std::string& name) const
{
  return const_cast<Node*>(this)->get_children(name);
}

Element* Node::add_child(const std::string& name,
                         const std::string& ns_prefix)
{
  _xmlNode* child = create_new_child_node(name, ns_prefix);
  if(!child)
    return 0;

  _xmlNode* node = xmlAddChild(impl_, child);
  if(node)
    return static_cast<Element*>(node->_private);
  else
     return 0;
}

Element* Node::add_child(xmlpp::Node* previous_sibling, 
                         const std::string& name,
                         const std::string& ns_prefix)
{
  if(!previous_sibling)
    return 0;

  _xmlNode* child = create_new_child_node(name, ns_prefix);
  if(!child)
    return 0;

  _xmlNode* node = xmlAddNextSibling(previous_sibling->cobj(), child);
  if(node)
    return static_cast<Element*>(node->_private);
  else
     return 0;
}

Element* Node::add_child_before(xmlpp::Node* next_sibling, 
                         const std::string& name,
                         const std::string& ns_prefix)
{
  if(!next_sibling)
    return 0;

  _xmlNode* child = create_new_child_node(name, ns_prefix);
  if(!child)
    return 0;

  _xmlNode* node = xmlAddPrevSibling(next_sibling->cobj(), child);
  if(node)
    return static_cast<Element*>(node->_private);
  else
     return 0;
}

_xmlNode* Node::create_new_child_node(const std::string& name, const std::string& ns_prefix)
{
   xmlNs* ns = 0;

   if(impl_->type != XML_ELEMENT_NODE)
   {
      #ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
      throw internal_error("You can only add child nodes to element nodes");
      #else
      return 0;
      #endif //LIBXMLCPP_EXCEPTIONS_ENABLED
   }

   //Ignore the namespace if none was specified:
   if(!ns_prefix.empty())
   {
     //Use the existing namespace if one exists:
     ns = xmlSearchNs(impl_->doc, impl_, (const xmlChar*)ns_prefix.c_str());
     if (!ns)
     {
       #ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
       throw exception("The namespace prefix (" + ns_prefix + ") has not been declared.");
       #else
       return 0;
       #endif //LIBXMLCPP_EXCEPTIONS_ENABLED
     }
   }

   return xmlNewNode(ns, (const xmlChar*)name.c_str());
}


void Node::remove_child(Node* node)
{
  //TODO: Allow a node to be removed without deleting it, to allow it to be moved?
  //This would require a more complex memory management API.
  xmlUnlinkNode(node->cobj());
  xmlFreeNode(node->cobj()); //The C++ instance will be deleted in a callback.
}

Node* Node::import_node(const Node* node, bool recursive)
{
  //Create the node, by copying:
  xmlNode* imported_node = xmlDocCopyNode(const_cast<xmlNode*>(node->cobj()), impl_->doc, recursive);
  if (!imported_node)
  {
    #ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
    throw exception("Unable to import node");
    #else
    return 0;
    #endif //LIBXMLCPP_EXCEPTIONS_ENABLED
  }

  //Add the node:
  xmlNode* added_node = xmlAddChild(this->cobj(),imported_node);
  if (!added_node)
  {
    xmlFreeNode(imported_node);

    #ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
    throw exception("Unable to add imported node to current node");
    #else
    return 0;
    #endif //LIBXMLCPP_EXCEPTIONS_ENABLED
  }

  return static_cast<Node*>(imported_node->_private);
}

std::string Node::get_name() const
{
  return impl_->name ? (const char*)impl_->name : "";
}

void Node::set_name(const std::string& name)
{
  xmlNodeSetName( impl_, (const xmlChar *)name.c_str() );
}

int Node::get_line() const
{
   return XML_GET_LINE(impl_);
}


xmlNode* Node::cobj()
{
  return impl_;
}

const xmlNode* Node::cobj() const
{
  return impl_;
}

std::string Node::get_path() const
{
  xmlChar* path = xmlGetNodePath(impl_);
  std::string retn = path ? (char*)path : "";
  xmlFree(path);
  return retn;
}

static NodeSet find_impl(xmlXPathContext* ctxt, const std::string& xpath)
{
  xmlXPathObject* result = xmlXPathEval((const xmlChar*)xpath.c_str(), ctxt);

  if(!result)
  {
    xmlXPathFreeContext(ctxt);

    #ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
    throw exception("Invalid XPath: " + xpath);
    #else
    return NodeSet();
    #endif //LIBXMLCPP_EXCEPTIONS_ENABLED
  }

  if(result->type != XPATH_NODESET)
  {
    xmlXPathFreeObject(result);
    xmlXPathFreeContext(ctxt);

    #ifdef LIBXMLCPP_EXCEPTIONS_ENABLE
    throw internal_error("Only nodeset result types are supported.");
    #else
    return NodeSet();
    #endif //LIBXMLCPP_EXCEPTIONS_ENABLED
  }

  xmlNodeSet* nodeset = result->nodesetval;
  NodeSet nodes;
  if( nodeset )
  {
    nodes.reserve( nodeset->nodeNr );
    for (int i = 0; i != nodeset->nodeNr; ++i)
      nodes.push_back(static_cast<Node*>(nodeset->nodeTab[i]->_private));
  }
  else
  {
    // return empty set
  }

  xmlXPathFreeObject(result);
  xmlXPathFreeContext(ctxt);

  return nodes;
}

NodeSet Node::find(const std::string& xpath) const
{
  xmlXPathContext* ctxt = xmlXPathNewContext(impl_->doc);
  ctxt->node = impl_;
  
  return find_impl(ctxt, xpath);
}

NodeSet Node::find(const std::string& xpath,
		   const PrefixNsMap& namespaces) const
{
  xmlXPathContext* ctxt = xmlXPathNewContext(impl_->doc);
  ctxt->node = impl_;

  for (PrefixNsMap::const_iterator it=namespaces.begin();
       it != namespaces.end(); it++)
    xmlXPathRegisterNs(ctxt,
		       reinterpret_cast<const xmlChar*>(it->first.c_str()),
		       reinterpret_cast<const xmlChar*>(it->second.c_str()));

  return find_impl(ctxt, xpath);
}

std::string Node::get_namespace_prefix() const
{
  if(impl_->type == XML_DOCUMENT_NODE)
  {
    //impl_ is actually of type xmlDoc, instead of just xmlNode.
    //libxml does not always use GObject-style inheritance, so xmlDoc does not have all the same struct fields as xmlNode.
    //Therefore, a call to impl_->ns would be invalid.
    //This can be an issue when calling this method on a Node returned by Node::find().
    //See the TODO comment on Document, suggesting that Document should derived from Node.

    return std::string();
  }

  if(impl_ && impl_->ns && impl_->ns->prefix)
    return (char*)impl_->ns->prefix;
  else
    return std::string();
}

std::string Node::get_namespace_uri() const
{
  if(impl_->type == XML_DOCUMENT_NODE)
  {
    //impl_ is actually of type xmlDoc, instead of just xmlNode.
    //libxml does not always use GObject-style inheritance, so xmlDoc does not have all the same struct fields as xmlNode.
    //Therefore, a call to impl_->ns would be invalid.
    //This can be an issue when calling this method on a Node returned by Node::find().
    //See the TODO comment on Document, suggesting that Document should derived from Node.

    return std::string();
  }

  if(impl_ && impl_->ns && impl_->ns->href)
    return (char*)impl_->ns->href;
  else
    return std::string();
}

void Node::set_namespace(const std::string& ns_prefix)
{
  //Look for the existing namespace to use:
  xmlNs* ns = xmlSearchNs( cobj()->doc, cobj(), (xmlChar*)(ns_prefix.empty() ? 0 : ns_prefix.c_str()) );
  if(ns)
  {
      //Use it for this element:
      xmlSetNs(cobj(), ns);
  }
  else
  {
    #ifdef LIBXMLCPP_EXCEPTIONS_ENABLE
    throw exception("The namespace (" + ns_prefix + ") has not been declared.");
    #endif //LIBXMLCPP_EXCEPTIONS_ENABLE
  }
}


} //namespace xmlpp
