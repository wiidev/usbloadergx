/* dtdvalidator.cpp
 * libxml++ and this file are copyright (C) 2000 by Ari Johnson
 * (C) 2002-2004 by the libxml dev team and
 * are covered by the GNU Lesser General Public License, which should be
 * included with libxml++ as the file COPYING.
 */

#include "libxml++/validators/schemavalidator.h"
#include "libxml++/schema.h"

#include <libxml/xmlschemas.h>
#include <libxml/xmlschemastypes.h>

#include <sstream>
#include <iostream>

namespace xmlpp
{

SchemaValidator::SchemaValidator()
: schema_(0)
, embbeded_shema_(false)
, ctxt_(0)
{
}

SchemaValidator::SchemaValidator(const std::string& file)
: schema_(0)
, embbeded_shema_(false)
, ctxt_(0)
{
  parse_file( file );
}

SchemaValidator::SchemaValidator(Document& document)
: schema_(0)
, embbeded_shema_(false)
, ctxt_(0)
{
  parse_document( document );
}

SchemaValidator::SchemaValidator(Schema* schema)
: schema_(schema)
, embbeded_shema_(false)
, ctxt_(0)
{
}

SchemaValidator::~SchemaValidator()
{
  release_underlying();
  Validator::release_underlying();
}

void SchemaValidator::parse_context(_xmlSchemaParserCtxt* context)
{
  release_underlying(); // Free any existing dtd.

  xmlSchema* schema = xmlSchemaParse( context );
  if ( ! schema )
   throw parse_error("Schema could not be parsed");

  schema->_private = new Schema(schema);

  schema_ = static_cast<Schema*>(schema->_private);
  embbeded_shema_ = true;
}

void SchemaValidator::parse_file(const std::string& filename)
{
  xmlSchemaParserCtxtPtr ctx = xmlSchemaNewParserCtxt( filename.c_str() );
  parse_context( ctx );
  xmlSchemaFreeParserCtxt( ctx );
}

void SchemaValidator::parse_memory(const std::string& contents)
{
  xmlSchemaParserCtxtPtr ctx = xmlSchemaNewMemParserCtxt( contents.c_str(), contents.size() );
  parse_context( ctx );
  xmlSchemaFreeParserCtxt( ctx );
}

void SchemaValidator::parse_document(Document& document)
{
  xmlSchemaParserCtxtPtr ctx = xmlSchemaNewDocParserCtxt( document.cobj() );
  parse_context( ctx );
  xmlSchemaFreeParserCtxt( ctx );
}

void SchemaValidator::set_schema(Schema* schema)
{
  release_underlying();
  schema_ = schema;
  embbeded_shema_ = false;
}

void SchemaValidator::release_underlying()
{
  if(ctxt_)
  {
    xmlSchemaFreeValidCtxt( ctxt_ );
    ctxt_ = 0;
  }
  if(schema_)
  {
    if(embbeded_shema_)
      delete schema_;
    schema_ = 0;
  }
}

SchemaValidator::operator bool() const
{
  return schema_ != 0;
}

Schema* SchemaValidator::get_schema()
{
  return schema_;
}

const Schema* SchemaValidator::get_schema() const
{
  return schema_;
}

void SchemaValidator::initialize_valid()
{
  xmlSchemaSetValidErrors(ctxt_, &callback_validity_error, &callback_validity_warning, this);

  //Clear these temporary buffers too:
  validate_error_.erase();
  validate_warning_.erase();
}


bool SchemaValidator::validate(const Document* doc)
{
  if (!doc)
    throw internal_error("Document pointer cannot be 0");

  if (!schema_)
    throw internal_error("Must have a schema to validate document");

  // A context is required at this stage only
  if (!ctxt_)
    ctxt_ = xmlSchemaNewValidCtxt( schema_->cobj() );

  if(!ctxt_)
  {
    throw internal_error("Couldn't create validating context");
  }

  initialize_valid();

  int res = xmlSchemaValidateDoc( ctxt_, (xmlDoc*)doc->cobj() );

  if(res != 0)
  {
    check_for_exception();
    throw validity_error("Document failed schema validation");
  }

  return res;
}

bool SchemaValidator::validate(const std::string& file)
{
  if (file.empty())
    throw internal_error("File path must not be empty");

  if (!schema_)
    throw internal_error("Must have a schema to validate document");

  // A context is required at this stage only
  if (!ctxt_)
    ctxt_ = xmlSchemaNewValidCtxt( schema_->cobj() );

  if(!ctxt_)
  {
    throw internal_error("Couldn't create validating context");
  }

  initialize_valid();

  int res = xmlSchemaValidateFile( ctxt_, file.c_str(), 0 );

  if(res != 0)
  {
    check_for_exception();
    throw validity_error("Document failed schema validation");
  }

  return res;
}

} // namespace xmlpp

