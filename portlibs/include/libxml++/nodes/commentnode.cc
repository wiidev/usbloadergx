/* commentnode.cc
 * libxml++ and this file are copyright (C) 2000 by Ari Johnson, and
 * are covered by the GNU Lesser General Public License, which should be
 * included with libxml++ as the file COPYING.
 */

#include <libxml++/nodes/commentnode.h>
#include <libxml++/exceptions/internal_error.h>

#include <libxml/tree.h>

namespace xmlpp
{

CommentNode::CommentNode(xmlNode* node)
: ContentNode(node)
{}

CommentNode::~CommentNode()
{}

} //namespace xmlpp

