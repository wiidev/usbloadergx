/* textnode.h
 * libxml++ and this file are copyright (C) 2000 by Ari Johnson, and
 * are covered by the GNU Lesser General Public License, which should be
 * included with libxml++ as the file COPYING.
 */

#ifndef __LIBXMLPP_NODES_TEXTNODE_H
#define __LIBXMLPP_NODES_TEXTNODE_H

#include <libxml++/nodes/contentnode.h>

namespace xmlpp
{

/** Text Node. This will be instantiated by the parser.
*/
class TextNode : public ContentNode
{
public:
  explicit TextNode(_xmlNode* node);
  virtual ~TextNode();
};

} // namespace xmlpp

#endif //__LIBXMLPP_NODES_TEXTNODE_H



