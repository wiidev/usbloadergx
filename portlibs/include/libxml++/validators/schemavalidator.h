/* schemavalidator.h
 * libxml++ and this file are copyright (C) 2000 by Ari Johnson,
 * (C) 2002-2004 by the libxml dev team and
 * are covered by the GNU Lesser General Public License, which should be
 * included with libxml++ as the file COPYING.
 */

#ifndef __LIBXMLPP_VALIDATOR_SCHEMAVALIDATOR_H
#define __LIBXMLPP_VALIDATOR_SCHEMAVALIDATOR_H

#include <libxml++/validators/validator.h>
#include <libxml++/schema.h>
#include <libxml++/document.h>

#ifndef DOXYGEN_SHOULD_SKIP_THIS
extern "C" {
  struct _xmlSchemaParserCtxt;
  struct _xmlSchemaValidCtxt;
}
#endif //DOXYGEN_SHOULD_SKIP_THIS4

namespace xmlpp {

/** XML DOM parser.
 *
 * @newin2p24
 */
class SchemaValidator : public Validator
{
public:
  SchemaValidator();
  explicit SchemaValidator(const std::string& file);
  explicit SchemaValidator(Document& document);
  explicit SchemaValidator(Schema* schema);
  virtual ~SchemaValidator();

  virtual void parse_file(const std::string& filename);
  virtual void parse_memory(const std::string& contents);
  virtual void parse_document(Document& document);
  virtual void set_schema(Schema* schema);

  /** Test whether a document has been parsed.
  */
  operator bool() const;
  Schema* get_schema();
  const Schema* get_schema() const;

  bool validate(const Document* doc);
  bool validate(const std::string& file);

protected:
  virtual void initialize_valid();
  void parse_context(_xmlSchemaParserCtxt* context);
  virtual void release_underlying();

  Schema* schema_;
  bool embbeded_shema_;
  _xmlSchemaValidCtxt* ctxt_;
};




} // namespace xmlpp

#endif //__LIBXMLPP_VALIDATOR_SCHEMAVALIDATOR_H

