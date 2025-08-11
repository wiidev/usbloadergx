/* document.h
 * this file is part of libxml++
 *
 * copyright (C) 2003 by libxml++ developer's team
 *
 * this file is covered by the GNU Lesser General Public License,
 * which should be included with libxml++ as the file COPYING.
 */

#include <libxml++/io/ostreamoutputbuffer.h>

namespace xmlpp
{
  OStreamOutputBuffer::OStreamOutputBuffer(
      std::ostream & output,
      const std::string& encoding)
    : OutputBuffer(encoding), output_(output)
  {
  }

  OStreamOutputBuffer::~OStreamOutputBuffer()
  {
  }
  
  bool OStreamOutputBuffer::do_write(
      const char * buffer,
      int len)
  {
    // here we rely on the ostream implicit conversion to boolean, to know if the stream can be used and/or if the write succeded.
    if(output_)
      output_.write(buffer, len);
    return !!output_;
  }

  bool OStreamOutputBuffer::do_close()
  {
    if(output_)
        output_.flush();
    return !!output_;
  }
}
