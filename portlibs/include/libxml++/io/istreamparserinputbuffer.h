/* inputstreamparserinputbuffer.h
 * this file is part of libxml++
 *
 * copyright (C) 2003 by libxml++ developer's team
 *
 * this file is covered by the GNU Lesser General Public License,
 * which should be included with libxml++ as the file COPYING.
 */

#ifndef __LIBXMLPP_ISTREAMPARSEROUTPUTBUFFER_H
#define __LIBXMLPP_ISTREAMPARSEROUTPUTBUFFER_H

#include <libxml++/io/parserinputbuffer.h>

#include <istream>

namespace xmlpp
{
  class IStreamParserInputBuffer: public ParserInputBuffer
  {
    public:
      /**
       * @param input The istream datas will be read from
       */
      IStreamParserInputBuffer(std::istream& input);
      virtual ~IStreamParserInputBuffer();

    private:
      virtual int  do_read(char * buffer, int len);
      virtual bool do_close();

      std::istream& input_;
  };
}

#endif
