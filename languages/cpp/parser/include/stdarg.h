/*
  Copyright 2005 Roberto Raggi <roberto@kdevelop.org>

  Permission to use, copy, modify, distribute, and sell this software and its
  documentation for any purpose is hereby granted without fee, provided that
  the above copyright notice appear in all copies and that both that
  copyright notice and this permission notice appear in supporting
  documentation.

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  KDEVELOP TEAM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
  AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef __STDARG
#define __STDARG

#if !defined(_VA_LIST) && !defined(__VA_LIST_DEFINED)
#define _VA_LIST
#define _VA_LIST_DEFINED

typedef char *__va_list;
#endif
static float __va_arg_tmp;
typedef __va_list va_list;

#define va_start(list, start)               ((void)0)
#define __va_arg(list, mode, n)             ((void)0)
#define _bigendian_va_arg(list, mode, n)    ((void)0)
#define _littleendian_va_arg(list, mode, n) ((void)0)
#define va_end(list)                        ((void)0)
#define va_arg(list, mode)                  ((void)0)

typedef void *__gnuc_va_list;

#endif
