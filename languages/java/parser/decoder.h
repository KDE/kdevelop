#ifndef JAVA_DECODER_H
#define JAVA_DECODER_H

#include "java_parser.h"

#include <string>
#include <cstdlib>

namespace java
{

class decoder
{
  parser::token_stream_type *_M_token_stream;

public:
  decoder(parser::token_stream_type *token_stream)
    : _M_token_stream(token_stream) {}

  int decode_op(std::size_t index) const
  {
    parser::token_type const &tk = _M_token_stream->token(index);
    return tk.kind;
  }

  std::string decode_id(std::size_t index) const
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

} // end of namespace java

#endif // JAVA_DECODER_H
