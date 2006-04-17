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

int main (int /*argc*/, char *argv[])
{
  bool no_stdinc = false;
  bool no_stdincpp = false;

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
#if defined (GCC_MACHINE) && defined (GCC_VERSION)
      preprocess.push_include_path ("/usr/lib/gcc/" GCC_MACHINE "/" GCC_VERSION "/include");
#endif
    }

  if (! no_stdincpp)
    {
#if defined (GCC_MACHINE) && defined (GCC_VERSION)
      preprocess.push_include_path ("/usr/include/c++/" GCC_VERSION);
      preprocess.push_include_path ("/usr/include/c++/" GCC_VERSION "/" GCC_MACHINE);
#endif
    }

  preprocess.push_include_path (".");

  preprocess.file ("pp-configuration", pp_null_output_iterator ()); // ### put your macros here!

  std::string result;
  result.reserve (20 * 1024); // 20K

  preprocess.file (filename, pp_output_iterator<std::string> (result));
  std::cout << result;

  // std::cout << "allocated #" << pp_symbol::count () << " symbols!" << std::endl;
  // std::cout << "allocated #" << std::distance (env.begin (), env.end ()) << " macros!" << std::endl;

  return EXIT_SUCCESS;
}

// kate: indent-width 2;
