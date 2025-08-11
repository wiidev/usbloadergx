/* noncopyable.h
 * libxml++ and this file are
 * copyright (C) 2000 by The libxml++ Development Team, and
 * are covered by the GNU Lesser General Public License, which should be
 * included with libxml++ as the file COPYING.
 */

#ifndef __LIBXMLPP_NONCOPYABLE_H
#define __LIBXMLPP_NONCOPYABLE_H

namespace xmlpp
{

/**
 * Herited by classes which cannot be copied.
 */
class NonCopyable
{
protected:
  NonCopyable();
  virtual ~NonCopyable();

private:
  //These are not-implemented.
  //They are just here so we can declare them as private
  //so that this, and any derived class, do not have
  //copy constructors.
  NonCopyable(const NonCopyable&);
  NonCopyable& operator=(const NonCopyable&);
};

} // namespace xmlpp

#endif //__LIBXMLPP_NONCOPYABLE_H

