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

#include "pp.cpp"

int main (int /*argc*/, char *argv[])
{
  char const *filename = *++argv;
  if (!filename)
    {
      std::cerr << "usage: pp file.cpp" << std::endl;
      return EXIT_FAILURE;
    }

  pp_environment env;

  pp<> preprocess(env);

  null_output_iterator null_out;
  preprocess.file ("pp-configuration", null_out); // ### put your macros here!

  std::string result;
  result.reserve (20 * 1024); // 20 K
  preprocess.file (filename, std::back_inserter (result));
  std::cout << result;

  return EXIT_SUCCESS;
}

