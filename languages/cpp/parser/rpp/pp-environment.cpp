/*
  Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#include "pp-environment.h"

#include <kdebug.h>

#include "pp-macro.h"
#include "pp-engine.h"

using namespace rpp;

Environment::Environment(pp* preprocessor)
  : m_replaying(false)
  , m_preprocessor(preprocessor)
{
}

Environment::~Environment()
{
}

MacroBlock* Environment::firstBlock() const
{
  if (!m_blocks.isEmpty())
    return m_blocks[0];

  return 0;
}

MacroBlock* Environment::currentBlock() const
{
  if (!m_blocks.isEmpty())
    return m_blocks.top();

  return 0;
}

void Environment::enterBlock(MacroBlock* block)
{
  if (!m_blocks.isEmpty())
    m_blocks.top()->childBlocks.append(block);

  m_blocks.push(block);
}

void Environment::visitBlock(MacroBlock* block, int depth)
{
  if (depth++ > 100) {
    // TODO detect recursion?
    kWarning() << k_funcinfo << "Likely cyclic include, aborting macro replay at depth 100" << endl;
    return;
  }

  if (!block->condition.isEmpty()) {
    Stream cs(&block->condition);
    Value result = m_preprocessor->eval_expression(cs);
    if (result.is_zero()) {
      if (block->elseBlock)
        visitBlock(block->elseBlock, depth);
      return;
    }
  }

  bool wasReplaying = m_replaying;
  m_replaying = true;

  int macroIndex = 0;
  int childIndex = 0;
  while (macroIndex < block->macros.count() || childIndex < block->childBlocks.count()) {
    MacroBlock* child = childIndex < block->childBlocks.count() ? block->childBlocks.at(childIndex) : 0;
    pp_macro* macro = macroIndex < block->macros.count() ? block->macros.at(macroIndex) : 0;

    Q_ASSERT(child || macro);

    bool visitMacro = macro && (!child || (child->sourceLine < macro->sourceLine));

    if (!visitMacro) {
      Q_ASSERT(child);
      visitBlock(child, depth);
      ++childIndex;

    } else {
      Q_ASSERT(macro);
      if (macro->defined)
        setMacro(macro);
      else
        clearMacro(macro->name);
      ++macroIndex;
    }
  }

  // No need to visit else block, it will be skipped (already a matched block)

  m_replaying = wasReplaying;
}

MacroBlock* Environment::enterBlock(int sourceLine, const QString& condition)
{
  MacroBlock* ret = new MacroBlock(sourceLine);
  ret->condition = condition;

  enterBlock(ret);

  return ret;
}

MacroBlock* Environment::elseBlock(int sourceLine, const QString& condition)
{
  MacroBlock* ret = new MacroBlock(sourceLine);
  ret->condition = condition;

  Q_ASSERT(!m_blocks.isEmpty());
  m_blocks.top()->elseBlock = ret;

  m_blocks.pop();
  m_blocks.push(ret);

  return ret;
}

void Environment::leaveBlock()
{
  m_blocks.pop();
}

void Environment::clear()
{
  m_environment.clear();
  m_blocks.clear();
}

void Environment::clearMacro(const QString& name)
{
  if (!m_replaying) {
    pp_macro* undef = new pp_macro();
    undef->name = name;
    undef->defined = false;

    m_blocks.top()->macros.append(undef);
  }

  m_environment.remove(name);
}

void Environment::setMacro(pp_macro* macro)
{
  if (!m_replaying)
    m_blocks.top()->macros.append(macro);

  m_environment.insert(macro->name, macro);
}

pp_macro* Environment::retrieveMacro(const QString& name) const
{
  if (m_environment.contains(name))
    return m_environment[name];

  return 0;
}

MacroBlock::MacroBlock(int _sourceLine)
  : elseBlock(0)
  , sourceLine(_sourceLine)
{
}

MacroBlock::~MacroBlock()
{
  qDeleteAll(macros);
  qDeleteAll(childBlocks);
  delete elseBlock;
}

void MacroBlock::setMacro(pp_macro* macro)
{
  macros.append(macro);
}

void rpp::Environment::cleanup()
{
  delete firstBlock();
  clear();
}

QList<pp_macro*> Environment::allMacros() const
{
  return m_environment.values();
}
