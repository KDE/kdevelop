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

#include <hashedstring.h> //Needs to be up here, so qHash(HashedString) is found

#include "pp-environment.h"

#include <kdebug.h>

#include "pp-macro.h"
#include "pp-engine.h"
#include "pp-location.h"

using namespace rpp;

Environment::Environment(pp* preprocessor)
  : m_replaying(false)
  , m_preprocessor(preprocessor)
  , m_locationTable(new LocationTable)
{
}

Environment::~Environment()
{
  delete m_locationTable;
}

LocationTable* Environment::locationTable() const
{
  return m_locationTable;
}

LocationTable* Environment::takeLocationTable()
{
  LocationTable* ret = m_locationTable;
  m_locationTable = new LocationTable;
  return ret;
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
    kDebug(9007) << "Likely cyclic include, aborting macro replay at depth 100" ;
    return;
  }

  if (!block->condition.isEmpty()) {
    Stream cs(&block->condition, Anchor(0,0));
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

MacroBlock* Environment::enterBlock(int sourceLine, const QByteArray& condition)
{
  MacroBlock* ret = new MacroBlock(sourceLine);
  ret->condition = condition;

  enterBlock(ret);

  return ret;
}

MacroBlock* Environment::elseBlock(int sourceLine, const QByteArray& condition)
{
  MacroBlock* ret = new MacroBlock(sourceLine);
  ret->condition = condition;

  Q_ASSERT(!m_blocks.isEmpty());
  m_blocks.top()->elseBlock = ret;

  m_blocks.pop();
  m_blocks.push(ret);

  return ret;
}

void Environment::swapMacros( Environment* parentEnvironment ) {
  EnvironmentMap oldEnvironment = m_environment;
  m_environment = parentEnvironment->m_environment;
  parentEnvironment->m_environment = oldEnvironment;
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

void Environment::clearMacro(const KDevelop::HashedString& name)
{
//   pp_macro* undef = new pp_macro();
//   undef->name = name;
//   undef->defined = false;
//   if(!m_replaying)
//     m_blocks.top()->macros.append(undef);
// 
//   setMacro(undef); //Before, m_environment.remove(..) was called

 if(!m_replaying) {
    pp_macro* undef = new pp_macro();
    undef->name = name;
    undef->defined = false;
    m_blocks.top()->macros.append(undef);
  }

  ///@todo Think about how this plays together with environment-management
  ///We need undef-macros to be put into the definedMacros etc. lists
  m_environment.remove(name);
}

void Environment::setMacro(pp_macro* macro)
{
  if (!m_replaying && !m_blocks.isEmpty())
    m_blocks.top()->macros.append(macro);

/*  if( !macro->defined )
    clearMacro(macro->name);
  else*/
    m_environment.insert(macro->name, macro);
}

const Environment::EnvironmentMap& Environment::environment() const {
  return m_environment;
}

pp_macro* Environment::retrieveStoredMacro(const KDevelop::HashedString& name) const
{
  if (m_environment.contains(name))
    return m_environment[name];

  return 0;
}

pp_macro* Environment::retrieveMacro(const QByteArray& name) const
{
  return retrieveMacro(QString::fromUtf8(name));
}

pp_macro* Environment::retrieveMacro(const KDevelop::HashedString& name) const
{
  return retrieveStoredMacro(name);
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
