/* commentnode.h
 * libxml++ and this file are copyright (C) 2000 by Ari Johnson, and
 * are covered by the GNU Lesser General Public License, which should be
 * included with libxml++ as the file COPYING.
 */

#ifndef __LIBXMLPP_NODES_COMMENTNODE_H
#define __LIBXMLPP_NODES_COMMENTNODE_H

#include <libxml++/nodes/contentnode.h>

namespace xmlpp
{

/** Comment Node. This will be instantiated by the parser.
 */
class CommentNode : public ContentNode
{
public:
  explicit CommentNode(_xmlNode* node);
  virtual ~CommentNode();
};

} // namespace xmlpp

#endif //__LIBXMLPP_NODES_COMMENTNODE_H



