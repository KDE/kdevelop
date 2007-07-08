/*
  This file is part of kdev-pg
  Copyright 2005, 2006 Roberto Raggi <roberto@kdevelop.org>

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

#ifndef KDEV_PG_REPLACEMENT_H
#define KDEV_PG_REPLACEMENT_H

#include <deque>
#include <string>

class replacement
{
public:
  virtual ~replacement() {}

  virtual std::size_t start_position() const = 0;
  virtual std::size_t size() const = 0;
  virtual char const *text() const = 0;
};

class text_replacement: public replacement
{
public:
  inline text_replacement(std::size_t start_position,
                          std::size_t size,
                          char const *text)
    : _M_start_position(start_position),
      _M_size(size),
      _M_text(text) {}

  virtual std::size_t start_position() const
  { return _M_start_position; }

  virtual std::size_t size() const
  { return _M_size; }

  virtual char const *text() const
  { return _M_text; }

private:
  std::size_t _M_start_position;
  std::size_t _M_size;
  char const *_M_text;
};

template <class _Tp, class _Tp1>
class replacement_list
{
  typedef _Tp1 token_t;

public:
  replacement_list(std::deque<token_t> const &token_stream);
  ~replacement_list();

  void set_contents(char const *contents)
  { _M_contents = contents; }

  void insert_text_before(_Tp *node, char const *text);
  void insert_text_after(_Tp *node, char const *text);

  void replace_text(_Tp *node, char const *text);
  void replace_text(std::size_t position, std::size_t length,
                    char const *text);

  void remove_text(_Tp *node);

  inline const std::deque<replacement*> &replacements() const
  { return _M_replacements; }

  void clear();
  std::string apply() const;

private:
  char const *_M_contents;
  std::deque<token_t> const &_M_token_stream;
  std::deque<replacement*> _M_replacements;
};

template <class _Tp, class _Tp1>
replacement_list<_Tp, _Tp1>::replacement_list(std::deque<token_t> const &token_stream)
  : _M_contents(0), _M_token_stream(token_stream)
{
}

template <class _Tp, class _Tp1>
replacement_list<_Tp, _Tp1>::~replacement_list()
{
  clear();
}

template <class _Tp, class _Tp1>
void replacement_list<_Tp, _Tp1>::clear()
{
  for (std::deque<replacement*>::iterator it = _M_replacements.begin();
       it != _M_replacements.end(); ++it)
    {
      delete *it;
    }

  _M_replacements.clear();
}

template <class _Tp, class _Tp1>
void replacement_list<_Tp, _Tp1>::insert_text_before(_Tp *node,
                                                     char const *text)
{
  std::size_t start_position = _M_token_stream[node->start_token].start;
  replace_text(start_position, 0, text);
}

template <class _Tp, class _Tp1>
void replacement_list<_Tp, _Tp1>::insert_text_after(_Tp *node,
                                                    char const *text)
{
  std::size_t end_position = _M_token_stream[node->end_token].start;
  replace_text(end_position, 0, text);
}

template <class _Tp, class _Tp1>
void replacement_list<_Tp, _Tp1>::replace_text(_Tp *node, char const *text)
{
  std::size_t start_position = _M_token_stream[node->start_token].start;
  std::size_t end_position = _M_token_stream[node->end_token].start;

  replace_text(start_position, end_position - start_position, text);
}

template <class _Tp, class _Tp1>
void replacement_list<_Tp, _Tp1>::remove_text(_Tp *node)
{
  replace_text(node, "");
}

namespace
{
  struct _Compare_replacement
  {
    bool operator()(replacement const *a, std::size_t start_position) const
    { return a->start_position() < start_position; }
  };
}

template <class _Tp, class _Tp1>
void replacement_list<_Tp, _Tp1>::replace_text(std::size_t position,
                                               std::size_t length, char const *text)
{
  std::deque<replacement*>::iterator __pos =
    std::lower_bound(_M_replacements.begin(), _M_replacements.end(), position, _Compare_replacement());

  replacement *r = new text_replacement(position, length, text);
  _M_replacements.insert(__pos, r);
}

template <class _Tp, class _Tp1>
std::string replacement_list<_Tp, _Tp1>::apply() const
{
  std::string source = _M_contents;
  std::string result;

  result.reserve(source.size() * 2);

  std::size_t pos = 0;

  for (int i=0; i<_M_replacements.size(); ++i)
    {
      replacement *repl = _M_replacements[i];

      result += std::string(source, pos, repl->start_position() - pos);
      result += repl->text();
      pos = repl->start_position() + repl->size();
    }

  assert(pos != 0);

  result += std::string(source, pos);

  return result;
}

#endif // KDEV_PG_REPLACEMENT_H
