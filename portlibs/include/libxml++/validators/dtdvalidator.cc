/* dtdvalidator.cpp
 * libxml++ and this file are copyright (C) 2000 by Ari Johnson
 * (C) 2002-2004 by the libxml dev team and
 * are covered by the GNU Lesser General Public License, which should be
 * included with libxml++ as the file COPYING.
 */

#include "libxml++/validators/dtdvalidator.h"
#include "libxml++/dtd.h"
#include "libxml++/nodes/element.h"
#include "libxml++/nodes/textnode.h"
#include "libxml++/nodes/commentnode.h"
#include "libxml++/keepblanks.h"
#include "libxml++/exceptions/internal_error.h"
#include "libxml++/io/istreamparserinputbuffer.h"

#include <libxml/parserInternals.h>//For xmlCreateFileParserCtxt().

#include <sstream>
#include <iostream>

namespace xmlpp
{

DtdValidator::DtdValidator()
: dtd_(0)
{
}

DtdValidator::DtdValidator(const std::string& file)
: dtd_(0)
{
  parse_subset("",file);
}

DtdValidator::DtdValidator(const std::string& external,const std::string& system)
: dtd_(0)
{
  parse_subset(external,system);
}

DtdValidator::~DtdValidator()
{
  release_underlying();
  Validator::release_underlying();
}

void DtdValidator::parse_file(const std::string& filename)
{
  parse_subset("",filename);
}

void DtdValidator::parse_subset(const std::string& external,const std::string& system)
{
  release_underlying(); // Free any existing dtd.

  xmlDtd* dtd = xmlParseDTD(
    external.empty() ? 0 : (const xmlChar *)external.c_str(),
    system.empty() ? 0 : (const xmlChar *)system.c_str());

  if (!dtd)
  {
    #ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
    throw parse_error("Dtd could not be parsed");
    #else
    return;
    #endif //LIBXMLCPP_EXCEPTIONS_ENABLED
  }

  dtd_ = static_cast<Dtd*>(dtd->_private);
}

void DtdValidator::parse_memory(const std::string& contents)
{
  // Prepare an istream with buffer
  std::istringstream is( contents );

  parse_stream( is );
}

void DtdValidator::parse_stream(std::istream& in)
{
  release_underlying(); //Free any existing document.

  IStreamParserInputBuffer ibuff( in );

  xmlDtd* dtd = xmlIOParseDTD( 0, ibuff.cobj(), XML_CHAR_ENCODING_UTF8 );

  if (!dtd)
  {
    #ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
    throw parse_error("Dtd could not be parsed");
    #else
    return;
    #endif //LIBXMLCPP_EXCEPTIONS_ENABLED
  }

  dtd_ = static_cast<Dtd*>(dtd->_private);
}

void DtdValidator::release_underlying()
{
  if(dtd_)
  {
    xmlFreeDtd(dtd_->cobj());
    dtd_ = 0;
  }
}

DtdValidator::operator bool() const
{
  return dtd_ != 0;
}

Dtd* DtdValidator::get_dtd()
{
  return dtd_;
}

const Dtd* DtdValidator::get_dtd() const
{
  return dtd_;
}

bool DtdValidator::validate(const Document* doc)
{
  // A context is required at this stage only
  if (!valid_)
    valid_ = xmlNewValidCtxt();

  if(!valid_)
  {
    #ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
    throw internal_error("Couldn't create parsing context");
    #else
    return false;
    #endif //LIBXMLCPP_EXCEPTIONS_ENABLED
  }

  if (!doc)
  {
    #ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
    throw internal_error("Document pointer cannot be 0");
    #else
    return false;
    #endif //LIBXMLCPP_EXCEPTIONS_ENABLED
  }

  initialize_valid();

  const bool res = (bool)xmlValidateDtd( valid_, (xmlDoc*)doc->cobj(), dtd_->cobj() );

  if(res == 0)
  {
    check_for_exception();

    #ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
    throw validity_error("Document failed Dtd validation");
    #else
    return false;
    #endif //LIBXMLCPP_EXCEPTIONS_ENABLED
  }

  return res;
}

} // namespace xmlpp

