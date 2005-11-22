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

class pp_environment: private std::map<pp_fast_string const *, pp_macro>
{
  typedef std::map<pp_fast_string const *, pp_macro>base_type;

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
};

#endif // PP_ENVIRONMENT_H
