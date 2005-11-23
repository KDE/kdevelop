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

#ifndef PP_CCTYPE_H
#define PP_CCTYPE_H

#include <cctype>

inline bool pp_isalpha (int ch)
{ return std::isalpha ((unsigned char) ch); }

inline bool pp_isalnum (int ch)
{ return std::isalnum ((unsigned char) ch); }

inline bool pp_isdigit (int ch)
{ return std::isdigit ((unsigned char) ch); }

inline bool pp_isspace (int ch)
{ return std::isspace ((unsigned char) ch); }

#endif // PP_CCTYPE_H

// kate: indent-width 2;
