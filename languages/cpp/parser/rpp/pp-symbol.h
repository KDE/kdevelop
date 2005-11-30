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

#ifndef PP_SYMBOL_H
#define PP_SYMBOL_H

class pp_symbol
{
  static rxx_allocator<char> &allocator_instance ()
  {
    static rxx_allocator<char>__allocator;
    return __allocator;
  }

public:
  static int &N()
  {
    static int __N;
    return __N;
  }

  static pp_fast_string const *get (char const *__data, std::size_t __size)
  {
    ++N();
    char *data = allocator_instance ().allocate (__size + 1);
    memcpy(data, __data, __size);
    data[__size] = '\0';

    char *where = allocator_instance ().allocate (sizeof (pp_fast_string));
    return new (where) pp_fast_string (data, __size);
  }

  template <typename _InputIterator>
  static pp_fast_string const *get (_InputIterator __first, _InputIterator __last)
  {
    ++N();
    std::ptrdiff_t __size = std::distance (__first, __last);
    assert (__size >= 0 && __size < 512);

    char *data = allocator_instance ().allocate (__size + 1);
    std::copy (__first, __last, data);
    data[__size] = '\0';

    char *where = allocator_instance ().allocate (sizeof (pp_fast_string));
    return new (where) pp_fast_string (data, __size);
  }

  static pp_fast_string const *get(std::string const &__s)
  { return get (__s.c_str (), __s.size ()); }
};

#endif // PP_SYMBOL_H

// kate: indent-width 2;
