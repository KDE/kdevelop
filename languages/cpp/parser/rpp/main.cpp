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

struct null_output_iterator
{
  typedef std::random_access_iterator_tag iterator_category;
  typedef char value_type;
  typedef char &reference;
  typedef char *pointer;
  typedef std::ptrdiff_t difference_type;
  char ch;

  null_output_iterator(): ch(0) {}

  char &operator * () { return ch; }
  null_output_iterator &operator ++ () { return *this; }
  null_output_iterator &operator ++ (int) { return *this; }
};

int main (int /*argc*/, char *argv[])
{
  bool no_stdinc = true;;
  bool no_stdincpp = true;

  char const *filename = *++argv;
  if (!filename)
    {
      std::cerr << "usage: pp file.cpp" << std::endl;
      return EXIT_FAILURE;
    }

  pp_environment env;

  pp<> preprocess(env);
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

  null_output_iterator null_out;
  preprocess.file ("pp-configuration", null_out); // ### put your macros here!

  std::string result;
  result.reserve (20 * 1024); // 20 K
  preprocess.file (filename, std::back_inserter (result));
  std::cout << result;

#if 0
  std::cerr << "============================= MACROS" << std::endl;
  for (pp_environment::iterator it = env.begin (); it != env.end (); ++it)
    {
      std::cerr << "#define " << (*it).first->begin () << " " << (*it).second.definition << std::endl;
    }
#endif

  return EXIT_SUCCESS;
}


