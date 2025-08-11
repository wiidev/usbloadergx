/* entityreference.h
 * libxml++ and this file are copyright (C) 2000 by Ari Johnson, and
 * are covered by the GNU Lesser General Public License, which should be
 * included with libxml++ as the file COPYING.
 */

#ifndef __LIBXMLPP_NODES_ENTITYREFERENCE_H
#define __LIBXMLPP_NODES_ENTITYREFERENCE_H

#include <libxml++/nodes/node.h>

namespace xmlpp
{

/** Entity references refer to previously declared entities. This will be instantiated by the parser.
 */
class EntityReference : public Node
{
public:
  explicit EntityReference(_xmlNode* node);
  virtual ~EntityReference();

  /** Get the text to which this entity reference would have resolved if the XML document had been parsed with Parser::set_substitute_entities(true).
   * @returns The unescaped text.
   */
  std::string get_resolved_text() const;

  //TODO: I'm not sure what this is. So far it seems to be the same as get_resolved_text().
  //      Maybe it's for nested entity declarations, though I don't know if that is even possile. murrayc.
  std::string get_original_text() const;

};

} // namespace xmlpp

#endif //__LIBXMLPP_NODES_TEXTNODE_H




