#ifndef MD5_H
#define MD5_H

#ifdef __cplusplus
extern "C"
{
#endif
/* ========================================================================== **
 *
 *                                    MD5.h
 *
 * Copyright:
 *  Copyright (C) 2003-2005 by Christopher R. Hertel
 *
 * Email: crh@ubiqx.mn.org
 *
 * $Id: MD5.h,v 0.6 2005/06/08 18:35:59 crh Exp $
 *
 * -------------------------------------------------------------------------- **
 *
 * Description:
 *  Implements the MD5 hash algorithm, as described in RFC 1321.
 *
 * -------------------------------------------------------------------------- **
 *
 * License:
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * -------------------------------------------------------------------------- **
 *
 * Notes:
 *
 *  None of this will make any sense unless you're studying RFC 1321 as you
 *  read the code.
 *
 *  MD5 is described in RFC 1321.
 *  The MD*4* algorithm is described in RFC 1320 (that's 1321 - 1).
 *  MD5 is very similar to MD4, but not quite similar enough to justify
 *  putting the two into a single module.  Besides, I wanted to add a few
 *  extra functions to this one to expand its usability.
 *
 *  There are three primary motivations for this particular implementation.
 *  1) Programmer's pride.  I wanted to be able to say I'd done it, and I
 *     wanted to learn from the experience.
 *  2) Portability.  I wanted an implementation that I knew to be portable
 *     to a reasonable number of platforms.  In particular, the algorithm is
 *     designed with little-endian platforms in mind, but I wanted an
 *     endian-agnostic implementation.
 *  3) Compactness.  While not an overriding goal, I thought it worth-while
 *     to see if I could reduce the overall size of the result.  This is in
 *     keeping with my hopes that this library will be suitable for use in
 *     some embedded environments.
 *  Beyond that, cleanliness and clarity are always worth pursuing.
 *
 *  As mentioned above, the code really only makes sense if you are familiar
 *  with the MD5 algorithm or are using RFC 1321 as a guide.  This code is
 *  quirky, however, so you'll want to be reading carefully.
 *
 *  Yeah...most of the comments are cut-and-paste from my MD4 implementation.
 *
 * -------------------------------------------------------------------------- **
 *
 * References:
 *  IETF RFC 1321: The MD5 Message-Digest Algorithm
 *       Ron Rivest. IETF, April, 1992
 *
 * ========================================================================== **
 */
/* -------------------------------------------------------------------------- **
 * Typedefs:
 */

typedef struct
  {
  uint32_t len;
  uint32_t ABCD[4];
  int      b_used;
  unsigned char    block[64];
  } auth_md5Ctx;


/* -------------------------------------------------------------------------- **
 * Functions:
 */

auth_md5Ctx *auth_md5InitCtx( auth_md5Ctx *ctx );
  /* ------------------------------------------------------------------------ **
   * Initialize an MD5 context.
   *
   *  Input:  ctx - A pointer to the MD5 context structure to be initialized.
   *                Contexts are typically created thusly:
   *                  ctx = (auth_md5Ctx *)malloc( sizeof(auth_md5Ctx) );
   *
   *  Output: A pointer to the initialized context (same as <ctx>).
   *
   *  Notes:  The purpose of the context is to make it possible to generate
   *          an MD5 Message Digest in stages, rather than having to pass a
   *          single large block to a single MD5 function.  The context
   *          structure keeps track of various bits of state information.
   *
   *          Once the context is initialized, the blocks of message data
   *          are passed to the <auth_md5SumCtx()> function.  Once the
   *          final bit of data has been handed to <auth_md5SumCtx()> the
   *          context can be closed out by calling <auth_md5CloseCtx()>,
   *          which also calculates the final MD5 result.
   *
   *          Don't forget to free an allocated context structure when
   *          you've finished using it.
   *
   *  See Also:  <auth_md5SumCtx()>, <auth_md5CloseCtx()>
   *
   * ------------------------------------------------------------------------ **
   */


auth_md5Ctx *auth_md5SumCtx( auth_md5Ctx *ctx,
                             const unsigned char *src,
                             const int    len );
  /* ------------------------------------------------------------------------ **
   * Build an MD5 Message Digest within the given context.
   *
   *  Input:  ctx - Pointer to the context in which the MD5 sum is being
   *                built.
   *          src - A chunk of source data.  This will be used to drive
   *                the MD5 algorithm.
   *          len - The number of bytes in <src>.
   *
   *  Output: A pointer to the updated context (same as <ctx>).
   *
   *  See Also:  <auth_md5InitCtx()>, <auth_md5CloseCtx()>, <auth_md5Sum()>
   *
   * ------------------------------------------------------------------------ **
   */


auth_md5Ctx *auth_md5CloseCtx( auth_md5Ctx *ctx, unsigned char *dst );
  /* ------------------------------------------------------------------------ **
   * Close an MD5 Message Digest context and generate the final MD5 sum.
   *
   *  Input:  ctx - Pointer to the context in which the MD5 sum is being
   *                built.
   *          dst - A pointer to at least 16 bytes of memory, which will
   *                receive the finished MD5 sum.
   *
   *  Output: A pointer to the closed context (same as <ctx>).
   *          You might use this to free a malloc'd context structure.  :)
   *
   *  Notes:  The context (<ctx>) is returned in an undefined state.
   *          It must be re-initialized before re-use.
   *
   *  See Also:  <auth_md5InitCtx()>, <auth_md5SumCtx()>
   *
   * ------------------------------------------------------------------------ **
   */


unsigned char * MD5(unsigned char * hash, const unsigned char *src, const int len );
  /* ------------------------------------------------------------------------ **
   * Compute an MD5 message digest.
   *
   *  Input:  dst - Destination buffer into which the result will be written.
   *                Must be 16 bytes, minimum.
   *          src - Source data block to be MD5'd.
   *          len - The length, in bytes, of the source block.
   *                (Note that the length is given in bytes, not bits.)
   *
   *  Output: A pointer to <dst>, which will contain the calculated 16-byte
   *          MD5 message digest.
   *
   *  Notes:  This function is a shortcut.  It takes a single input block.
   *          For more drawn-out operations, see <auth_md5InitCtx()>.
   *
   *          This function is interface-compatible with the
   *          <auth_md4Sum()> function in the MD4 module.
   *
   *          The MD5 algorithm is designed to work on data with an
   *          arbitrary *bit* length.  Most implementations, this one
   *          included, handle the input data in byte-sized chunks.
   *
   *          The MD5 algorithm does much of its work using four-byte
   *          words, and so can be tuned for speed based on the endian-ness
   *          of the host.  This implementation is intended to be
   *          endian-neutral, which may make it a teeny bit slower than
   *          others.  ...maybe.
   *
   *  See Also:  <auth_md5InitCtx()>
   *
   * ------------------------------------------------------------------------ **
   */

unsigned char * MD5fromFile(unsigned char *dst, const char *src);
  /* ------------------------------------------------------------------------ **
   * Compute an MD5 message digest.
   *
   *  Input:  dst - Destination buffer into which the result will be written.
   *                Must be 16 bytes, minimum.
   *          src - filepath to the file to be MD5'd.
   *
   *  Output: A pointer to <dst>, which will contain the calculated 16-byte
   *          MD5 message digest.
   *
   *  Notes:  This function is a shortcut.  It takes a single input block.
   *          For more drawn-out operations, see <auth_md5InitCtx()>.
   *
   *          This function is interface-compatible with the
   *          <auth_md4Sum()> function in the MD4 module.
   *
   *          The MD5 algorithm is designed to work on data with an
   *          arbitrary *bit* length.  Most implementations, this one
   *          included, handle the input data in byte-sized chunks.
   *
   *          The MD5 algorithm does much of its work using four-byte
   *          words, and so can be tuned for speed based on the endian-ness
   *          of the host.  This implementation is intended to be
   *          endian-neutral, which may make it a teeny bit slower than
   *          others.  ...maybe.
   *
   *  See Also:  <auth_md5InitCtx()>
   *
   * ------------------------------------------------------------------------ **
   */


/* ========================================================================== */

#ifdef __cplusplus
}
#endif
#endif /* AUTH_MD5_H */
