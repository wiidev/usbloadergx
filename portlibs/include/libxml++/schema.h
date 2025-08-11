/* schema.h
 * libxml++ and this file are copyright (C) 2000 by Ari Johnson, and
 * are covered by the GNU Lesser General Public License, which should be
 * included with libxml++ as the file COPYING.
 */

#ifndef __LIBXMLPP_SCHEMA_H
#define __LIBXMLPP_SCHEMA_H

#include <libxml++/attribute.h>
#include <libxml++/document.h>
#include <list>
#include <map>

#ifndef DOXYGEN_SHOULD_SKIP_THIS
extern "C" {
  struct _xmlSchema;
}
#endif //DOXYGEN_SHOULD_SKIP_THIS4

namespace xmlpp
{

/** Represents XML Schema.
 *
 * @newin2p24
 */
class Schema : NonCopyable
{
public:
  /** Create a schema from the underlying libxml schema element.
   */
  explicit Schema(_xmlSchema* schema);

  /** Create a schema from a XML document.
   * @param document XMLSchema document, NULL to create an empty schema document.
   * @param embed If true, the document will be deleted when
   *   the schema is deleted or another document is set.
   */
  explicit Schema(Document* document = NULL, bool embed = false);
  ~Schema();

  /** Set a new document to the schema.
   * @param document XMLSchema document, NULL to create an empty schema document.
   * @param embed If true, the document will be deleted when the schema is deleted or another document is set.
   */
  virtual void set_document(Document* document = NULL, bool embed = false);

  std::string get_name() const;
  std::string get_target_namespace() const;
  std::string get_version() const;

  Document* get_document();
  const Document* get_document()const;
  
  /** Access the underlying libxml implementation. */
  _xmlSchema* cobj();

  /** Access the underlying libxml implementation. */
  const _xmlSchema* cobj() const;

protected:
  virtual void release_underlying();

private:
  _xmlSchema* impl_;

  /** Is the base document is created with the schema. */
  bool embedded_doc_;
};

} // namespace xmlpp

#endif //__LIBXMLPP_SCHEMA_H



