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

  static std::set<pp_fast_string> &string_set_instance ()
  {
    static std::set<pp_fast_string> __string_set;
    return __string_set;
  }

public:
  static pp_fast_string const *get (char const *__data, std::size_t __size)
  {
    std::set<pp_fast_string> &string_set = string_set_instance ();

    std::set<pp_fast_string>::iterator it = string_set.find (pp_fast_string (__data, __size));
    if (it == string_set.end ())
      {
        char *where = allocator_instance ().allocate (__size + 1);
        memcpy(where, __data, __size);
        where[__size] = '\0';
        pp_fast_string str (where, __size);
        it = string_set.insert (str).first;
      }

    return &*it;
  }

  inline static bool used (pp_fast_string const *__s)
  {
    std::set<pp_fast_string> const &string_set = string_set_instance ();

    return string_set.find (*__s) != string_set.end ();
  }

  template <typename _InputIterator>
  static pp_fast_string const *get (_InputIterator __first, _InputIterator __last)
  {
    std::ptrdiff_t __size = std::distance (__first, __last);
    assert (__size >= 0 && __size < 512);

    char buffer[512], *cp = buffer;
    std::copy (__first, __last, cp);
    return get (buffer, __size);
  }

  static pp_fast_string const *get(std::string const &__s)
  { return get (__s.c_str (), __s.size ()); }
};

#endif // PP_SYMBOL_H
