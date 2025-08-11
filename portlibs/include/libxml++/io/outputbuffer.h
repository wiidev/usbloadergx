/* outputbuffer.h
 * this file is part of libxml++
 *
 * copyright (C) 2003 by libxml++ developer's team
 *
 * this file is covered by the GNU Lesser General Public License,
 * which should be included with libxml++ as the file COPYING.
 */

#ifndef __LIBXMLPP_OUTPUTBUFFER_H
#define __LIBXMLPP_OUTPUTBUFFER_H

#include <string>
#include <libxml++/noncopyable.h>

#ifndef DOXYGEN_SHOULD_SKIP_THIS
extern "C"
{
  struct _xmlOutputBuffer;
}
#endif //DOXYGEN_SHOULD_SKIP_THIS

namespace xmlpp
{
  struct OutputBufferCallback;

  /** Base class for xmlOutputBuffer wrapper
   *
   * It can be herited to create a new output buffer.
   * A child class has to override do_write, and eventually
   * do_close if some actions are required before buffer closing.
   */
  class OutputBuffer: public NonCopyable
  {
    public:
      /**
       * @param encoding The encoding herited class wait for in do_write. If
       * not provided, UTF-8 will be sent to do_write.
       *
       * @warning The encoding is done by libxml. As a consequence, libxml must
       * have a translator to the target encoding.
       */
      OutputBuffer(const std::string& encoding = std::string());
      virtual ~OutputBuffer();

    public:
      /** gives an access to the underlying libxml structure to the children
       */
      _xmlOutputBuffer* cobj();

      /** gives an access to the underlying libxml structure to the children
       */
      const _xmlOutputBuffer* cobj() const;

    private:
      bool on_write(const char * buffer, int len);
      bool on_close();

      /** Function called when some data are sent to the buffer.
       * @param buffer The datas encoded in the charset given to the constructor
       * @param len Buffer length
       *
       * This function MUST be overriden in herited classes.
       */
      virtual bool do_write(const char * buffer, int len) = 0;

      /** Function called before closing the buffer.
       * Herited classes should override it if some actions are required before
       * closing the buffer, instead of doing them in the destructor.
       */
      virtual bool do_close();

      /**
       * Underlying libxml2 structure.
       */
      _xmlOutputBuffer* impl_;

      friend struct OutputBufferCallback;
  };

}

#endif
