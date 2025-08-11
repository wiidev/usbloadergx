/* textnode.cc
 * libxml++ and this file are copyright (C) 2000 by Ari Johnson, and
 * are covered by the GNU Lesser General Public License, which should be
 * included with libxml++ as the file COPYING.
 */

#include <libxml++/nodes/textnode.h>
#include <libxml++/exceptions/internal_error.h>

#include <libxml/tree.h>

namespace xmlpp
{
  
TextNode::TextNode(xmlNode* node)
: ContentNode(node)
{}

TextNode::~TextNode()
{}

} //namespace xmlpp
