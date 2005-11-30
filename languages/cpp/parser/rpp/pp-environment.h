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

#ifndef PP_ENVIRONMENT_H
#define PP_ENVIRONMENT_H

#ifdef HAVE_TR1
#  include <tr1/unordered_map>
#endif

class pp_environment
#ifdef HAVE_TR1
  : private std::tr1::unordered_map<pp_fast_string const *, pp_macro, _PP_internal::_Hash_string, _PP_internal::_Equal_to_string>
#else
  : private std::map<pp_fast_string const *, pp_macro, _PP_internal::_Compare_string>
#endif
{
#ifdef HAVE_TR1
  typedef std::tr1::unordered_map<pp_fast_string const *, pp_macro, _PP_internal::_Hash_string, _PP_internal::_Equal_to_string> base_type;
#else
  typedef std::map<pp_fast_string const *, pp_macro, _PP_internal::_Compare_string> base_type;
#endif

public:
  using base_type::begin;
  using base_type::end;
  using base_type::iterator;
  using base_type::const_iterator;

  inline bool bind (pp_fast_string const *__name, pp_macro const &__macro)
  { return insert (std::make_pair (__name, __macro)).second; }

  inline void unbind (pp_fast_string const *__name)
  { erase (__name); }

  inline pp_macro *resolve (pp_fast_string const *__name)
  {
    iterator it = find (__name);
    return it != end () ? &(*it).second : 0;
  }

  inline pp_macro *resolve (char const *__data, std::size_t __size)
  {
    pp_fast_string __tmp (__data, __size);
    return resolve (&__tmp);
  }
};

#endif // PP_ENVIRONMENT_H

// kate: indent-width 2;
