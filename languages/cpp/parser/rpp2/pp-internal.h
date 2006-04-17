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

#ifndef PP_INTERNAL_H
#define PP_INTERNAL_H

namespace _PP_internal
{

template <typename _InputIterator>
inline bool comment_p (_InputIterator __first, _InputIterator __last) /*const*/
{
    if (__first == __last)
    return false;

    if (*__first != '/')
    return false;

    if (++__first == __last)
    return false;

    return (*__first == '/' || *__first == '*');
}

struct _Compare_string: public std::binary_function<bool, pp_fast_string const *, pp_fast_string const *>
{
  inline bool operator () (pp_fast_string const *__lhs, pp_fast_string const *__rhs) const
  { return *__lhs < *__rhs; }
};

struct _Equal_to_string: public std::binary_function<bool, pp_fast_string const *, pp_fast_string const *>
{
  inline bool operator () (pp_fast_string const *__lhs, pp_fast_string const *__rhs) const
  { return *__lhs == *__rhs; }
};

struct _Hash_string: public std::unary_function<std::size_t, pp_fast_string const *>
{
  inline std::size_t operator () (pp_fast_string const *__s) const
  {
    char const *__ptr = __s->begin ();
    int __size = __s->size ();
    std::size_t __h = 0;

    while (--__size >= 0)
      __h = (__h << 5) - __h + *__ptr++;

    return __h;
  }
};

} // _PP_internal

#endif // PP_INTERNAL_H
