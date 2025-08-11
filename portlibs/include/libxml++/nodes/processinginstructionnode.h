/* processinginstructionnode.h
 * libxml++ and this file are copyright (C) 2000 by Ari Johnson, and
 * are covered by the GNU Lesser General Public License, which should be
 * included with libxml++ as the file COPYING.
 */

#ifndef __LIBXMLPP_NODES_PINODE_H
#define __LIBXMLPP_NODES_PINODE_H

#include <libxml++/nodes/contentnode.h>

namespace xmlpp
{

class ProcessingInstructionNode : public ContentNode
{
public:
  explicit ProcessingInstructionNode(_xmlNode* node);
  virtual ~ProcessingInstructionNode();
};

} // namespace xmlpp

#endif //__LIBXMLPP_NODES_TEXTNODE_H

