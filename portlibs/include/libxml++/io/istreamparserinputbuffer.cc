/* istreamparserinputbuffer
 * this file is part of libxml++
 *
 * copyright (C) 2003 by libxml++ developer's team
 *
 * this file is covered by the GNU Lesser General Public License,
 * which should be included with libxml++ as the file COPYING.
 */

#include <libxml++/io/istreamparserinputbuffer.h>

namespace xmlpp
{
  IStreamParserInputBuffer::IStreamParserInputBuffer(
      std::istream & input)
    : ParserInputBuffer(), input_(input)
  {
  }

  IStreamParserInputBuffer::~IStreamParserInputBuffer()
  {
  }

  int IStreamParserInputBuffer::do_read(
      char * buffer,
      int len)
  {
    int l=0;
    if(input_)
    {
      // This is the correct statement - but gcc 2.95.3 lacks this method
      //l = input_.readsome(buffer, len);
      input_.read(buffer, len);
      l = input_.gcount();
    }

    return l;
  }

  bool IStreamParserInputBuffer::do_close()
  {
    return !!input_;
  }
}
