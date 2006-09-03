/*****************************************************************************
 * Copyright (c) 2006 Alexander Dymo <adymo@kdevelop.org>                    *
 *                                                                           *
 * Permission is hereby granted, free of charge, to any person obtaining     *
 * a copy of this software and associated documentation files (the           *
 * "Software"), to deal in the Software without restriction, including       *
 * without limitation the rights to use, copy, modify, merge, publish,       *
 * distribute, sublicense, and/or sell copies of the Software, and to        *
 * permit persons to whom the Software is furnished to do so, subject to     *
 * the following conditions:                                                 *
 *                                                                           *
 * The above copyright notice and this permission notice shall be            *
 * included in all copies or substantial portions of the Software.           *
 *                                                                           *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,           *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF        *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                     *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE    *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION    *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION     *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.           *
 *****************************************************************************/

#ifndef RUBY_DECODER_H
#define RUBY_DECODER_H

#include "ruby_parser.h"

#include <string>
#include <cstdlib>

namespace ruby
{

class decoder
{
  parser::token_stream_type *_M_token_stream;

public:
  decoder(parser::token_stream_type *token_stream)
    : _M_token_stream(token_stream) {}

  int decode_kind(std::size_t index) const
  {
    parser::token_type const &tk = _M_token_stream->token(index);
    return tk.kind;
  }

  std::string decode_string(std::size_t index) const
  {
    parser::token_type const &tk = _M_token_stream->token(index);
    return std::string(&tk.text[tk.begin], tk.end - tk.begin);
  }

  long decode_number(std::size_t index) const
  {
    parser::token_type const &tk = _M_token_stream->token(index);
    return ::strtol(&tk.text[tk.begin], 0, 0);
  }
};

} // end of namespace ruby

#endif // RUBY_DECODER_H

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
