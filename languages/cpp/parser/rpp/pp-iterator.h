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

#ifndef PP_ITERATOR_H
#define PP_ITERATOR_H

#include <iterator>

class pp_null_output_iterator
  : public std::iterator<std::output_iterator_tag, void, void, void, void>
{
public:
  pp_null_output_iterator() {}

  template <typename _Tp>
  pp_null_output_iterator &operator=(_Tp const &__value)
  { return *this; }

  inline pp_null_output_iterator &operator * () { return *this; }
  inline pp_null_output_iterator &operator ++ () { return *this; }
  inline pp_null_output_iterator operator ++ (int) { return *this; }
};

template <typename _Container>
class pp_output_iterator
  : public std::iterator<std::output_iterator_tag, void, void, void, void>
{
  std::string &_M_result;

public:
  explicit pp_output_iterator(std::string &__result):
    _M_result (__result) {}

  inline pp_output_iterator &operator=(typename _Container::const_reference __value)
  {
    if (_M_result.capacity () == _M_result.size ())
      _M_result.reserve (_M_result.capacity () << 2);

    _M_result.push_back(__value);
    return *this;
  }

  inline pp_output_iterator &operator * () { return *this; }
  inline pp_output_iterator &operator ++ () { return *this; }
  inline pp_output_iterator operator ++ (int) { return *this; }
};

#endif // PP_ITERATOR_H
