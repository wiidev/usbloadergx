/* cdatanode.h
 * libxml++ and this file are copyright (C) 2000 by Ari Johnson, and
 * are covered by the GNU Lesser General Public License, which should be
 * included with libxml++ as the file COPYING.
 */

#ifndef __LIBXMLPP_NODES_CDATANODE_H
#define __LIBXMLPP_NODES_CDATANODE_H

#include <libxml++/nodes/contentnode.h>

namespace xmlpp
{

/** CData node. This will be instantiated by the parser.
 *
 */
class CdataNode : public ContentNode
{
public:
  explicit CdataNode(_xmlNode* node);
  virtual ~CdataNode();
};

} // namespace xmlpp

#endif //__LIBXMLPP_NODES_TEXTNODE_H




