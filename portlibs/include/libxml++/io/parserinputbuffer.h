/* parserinputbuffer.h
 * this file is part of libxml++
 *
 * copyright (C) 2003 by libxml++ developer's team
 *
 * this file is covered by the GNU Lesser General Public License,
 * which should be included with libxml++ as the file COPYING.
 */

#ifndef __LIBXMLPP_PARSERINPUTBUFFER_H
#define __LIBXMLPP_PARSERINPUTBUFFER_H

#include <string>
#include <libxml++/noncopyable.h>

extern "C"
{
  struct _xmlParserInputBuffer;
}

namespace xmlpp
{
  struct ParserInputBufferCallback;

  /** Base class for xmlParserInputBuffer wrapper
   *
   * It can be herited to create a new output buffer.
   * A child class has to override do_write, and eventually
   * do_close if some actions are required before buffer closing.
   */
  class ParserInputBuffer: public NonCopyable
  {
    public:
      ParserInputBuffer();
      virtual ~ParserInputBuffer();

    public:
      /** gives an access to the underlying libxml structure to the children
       */
      _xmlParserInputBuffer* cobj();

      /** gives an access to the underlying libxml structure to the children
       */
      const _xmlParserInputBuffer* cobj() const;

    private:
      int on_read(char * buffer, int len);
      bool on_close();

      /** Function called when some data are read from the buffer.
       * @param buffer The datas encoded in the charset given to the constructor
       * @param len bytes to read
       * @return Number of bytes read
       *
       * This function MUST be overriden in herited classes.
       */
      virtual int do_read(char * buffer, int len) = 0;

      /** Function called before closing the buffer.
       * Herited classes should override it if some actions are required before
       * closing the buffer, instead of doing them in the destructor.
       */
      virtual bool do_close();

      /**
       * Underlying libxml2 structure.
       */
      _xmlParserInputBuffer* impl_;

      friend struct ParserInputBufferCallback;
  };

}

#endif
