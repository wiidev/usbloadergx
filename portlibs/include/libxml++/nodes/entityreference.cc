/* entityreference.cc
 * libxml++ and this file are copyright (C) 2000 by Ari Johnson, and
 * are covered by the GNU Lesser General Public License, which should be
 * included with libxml++ as the file COPYING.
 */

#include <libxml++/nodes/entityreference.h>
#include <libxml++/exceptions/internal_error.h>

#include <libxml/tree.h>

namespace xmlpp
{
  
EntityReference::EntityReference(xmlNode* node)
: Node(node)
{}

EntityReference::~EntityReference()
{}

std::string EntityReference::get_resolved_text() const
{
  std::string result;

  //Get the child xmlEntity node (there should only be 1).
  xmlNode* cChild = cobj()->children;
  if(cChild && cChild->type == XML_ENTITY_DECL)
  {
      xmlEntity* cEntity = (xmlEntity*)cChild;
      const xmlChar* pch = cEntity->content;
      if(pch)
        result = (const char*)pch;
  }
      
  return result;
}

std::string EntityReference::get_original_text() const
{
  std::string result;

  //Get the child xmlEntity node (there should only be 1).
  xmlNode* cChild = cobj()->children;
  if(cChild && cChild->type == XML_ENTITY_DECL)
  {
      xmlEntity* cEntity = (xmlEntity*)cChild;
      const xmlChar* pch = cEntity->orig;
      if(pch)
        result = (const char*)pch;
  }

  return result;
}


} //namespace xmlpp

