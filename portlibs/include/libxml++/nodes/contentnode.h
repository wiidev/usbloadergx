/* node.h
 * libxml++ and this file are copyright (C) 2000 by Ari Johnson, and
 * are covered by the GNU Lesser General Public License, which should be
 * included with libxml++ as the file COPYING.
 */

#ifndef __LIBXMLPP_NODES_CONTENTNODE_H
#define __LIBXMLPP_NODES_CONTENTNODE_H

#include <libxml++/nodes/node.h>

namespace xmlpp
{

/** Content Node. This will be instantiated by the parser.
 */
class ContentNode : public Node
{
public:
  explicit ContentNode(_xmlNode* node);
  virtual ~ContentNode();

  /** Get the text of this content node.
   * @returns The text. Note that the 5 predefined entities (&amp;, &quot;, &lt;, &qt, &apos;)
   * are always resolved, so this content will show their human-readable equivalents.
   */
  std::string get_content() const;

  /** Set the text of this content node
   * @param content The text. This must be unescaped, meaning that the predefined entities will be created for you where necessary.
   * See get_content().
   */
  void set_content(const std::string& content);

  /// @returns Whether this node contains only white space, or is empty.
  bool is_white_space() const;
};

} // namespace xmlpp

#endif //__LIBXMLPP_NODES_TEXTNODE_H




