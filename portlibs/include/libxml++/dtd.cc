/* dtd.cc
 * libxml++ and this file are copyright (C) 2000 by Ari Johnson, and
 * are covered by the GNU Lesser General Public License, which should be
 * included with libxml++ as the file COPYING.
 */

#include <libxml++/dtd.h>

#include <libxml/tree.h>

namespace xmlpp
{
  
Dtd::Dtd(_xmlDtd* dtd)
: impl_(dtd)
{
  dtd->_private = this;
}

Dtd::~Dtd()
{ 
}

std::string Dtd::get_name() const
{
  return (char*)impl_->name;
}

std::string Dtd::get_external_id() const
{
  return (char*)impl_->ExternalID;
}

std::string Dtd::get_system_id() const
{
  return (char*)impl_->SystemID;
}

_xmlDtd* Dtd::cobj()
{
  return impl_;
}

const _xmlDtd* Dtd::cobj() const
{
  return impl_;
}

} //namespace xmlpp
