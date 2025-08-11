// -*- C++ -*-

/* exception.h
 *
 * Copyright (C) 2002 The libxml++ development team
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __LIBXMLPP_EXCEPTION_H
#define __LIBXMLPP_EXCEPTION_H

#include <exception>
#include <string>

#include <libxml++config.h>

namespace xmlpp
{

/** Base class for all xmlpp exceptions.
 */
class LIBXMLPP_API exception: public std::exception
{
public:
  explicit exception(const std::string& message);
  virtual ~exception() throw();

  virtual const char* what() const throw();
  virtual void Raise() const;
  virtual exception * Clone() const;

private:
  std::string message_;
};

} // namespace xmlpp

#endif // __LIBXMLPP_EXCEPTION_H
