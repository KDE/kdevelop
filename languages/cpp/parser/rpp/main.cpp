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

#include "pp.h"
#include <iterator>

class null_output_iterator
  : public std::iterator<std::output_iterator_tag, void, void, void, void>
{
public:
  null_output_iterator() {}

  template <typename _Tp>
  null_output_iterator &operator=(_Tp const &__value)
  { return *this; }

  inline null_output_iterator &operator * () { return *this; }
  inline null_output_iterator &operator ++ () { return *this; }
  inline null_output_iterator operator ++ (int) { return *this; }
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

int main (int /*argc*/, char *argv[])
{
  bool no_stdinc = true;
  bool no_stdincpp = true;

  char const *filename = *++argv;
  if (!filename)
    {
      std::cerr << "usage: pp file.cpp" << std::endl;
      return EXIT_FAILURE;
    }

  pp_environment env;

  pp preprocess(env);
  if (! no_stdinc)
    {
      preprocess.push_include_path ("/usr/include");
      preprocess.push_include_path ("/usr/lib/gcc/" GCC_MACHINE "/" GCC_VERSION "/include");
    }

  if (! no_stdincpp)
    {
      preprocess.push_include_path ("/usr/include/c++/" GCC_VERSION);
      preprocess.push_include_path ("/usr/include/c++/" GCC_VERSION "/" GCC_MACHINE);
    }

  preprocess.push_include_path (".");

  preprocess.file ("pp-configuration", null_output_iterator ()); // ### put your macros here!

  std::string result;
  result.reserve (20 * 1024); // 20K

  preprocess.file (filename, pp_output_iterator<std::string> (result));
  std::cout << result;

  // std::cout << "allocated #" << pp_symbol::count () << " symbols!" << std::endl;
  // std::cout << "allocated #" << std::distance (env.begin (), env.end ()) << " macros!" << std::endl;

  return EXIT_SUCCESS;
}

// kate: indent-width 2;
