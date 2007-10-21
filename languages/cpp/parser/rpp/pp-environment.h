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

#ifndef PP_ENVIRONMENT_H
#define PP_ENVIRONMENT_H

#include <QHash>
#include <QMap>

#include <QStack>
#include <cppparserexport.h>

namespace KDevelop {
  class HashedString;
}

namespace rpp {

class pp_macro;
class pp;

class KDEVCPPRPP_EXPORT MacroBlock
{
public:
  MacroBlock(int _sourceLine);
  virtual ~MacroBlock();

  void setMacro(pp_macro* macro);

  QList<MacroBlock*> childBlocks;

  // The condition that opened this block
  QString condition;
  // The block to use if this block's condition was not met
  MacroBlock* elseBlock;

  // The source line where the block occurred
  int sourceLine;

  // This block is the owner of these macros
  QList<pp_macro*> macros;
};

class KDEVCPPRPP_EXPORT Environment
{
public:
  typedef QHash<KDevelop::HashedString, pp_macro*> EnvironmentMap;

  Environment(pp* preprocessor);
  virtual ~Environment();

  MacroBlock* firstBlock() const;
  MacroBlock* currentBlock() const;

  void enterBlock(MacroBlock* block);
  MacroBlock* enterBlock(int sourceLine, const QString& condition);
  MacroBlock* elseBlock(int sourceLine, const QString& condition = QString());
  void leaveBlock();

  // Replay previously saved blocks on this environment
  void visitBlock(MacroBlock* block, int depth = 0);

  void clear();

  // For those not interested in the result, just in getting memory released etc.
  void cleanup();

  void clearMacro(const KDevelop::HashedString& name);

  //Note: Undef-macros are allowed too
  virtual void setMacro(pp_macro* macro);
  virtual pp_macro* retrieveMacro(const KDevelop::HashedString& name) const;

  //Returns macros that are really stored locally(retrieveMacro may be overridden to perform more complex actions)
  pp_macro* retrieveStoredMacro(const KDevelop::HashedString& name) const;
  
  QList<pp_macro*> allMacros() const;

  //Take the set of environment-macros from the given environment
  void swapMacros( Environment* parentEnvironment );

  //Faster access then allMacros(..), because nothing is copied
  const EnvironmentMap& environment() const; //krazy:exclude=constref

private:
  EnvironmentMap m_environment;

  QStack<MacroBlock*> m_blocks;
  bool m_replaying;
  pp* m_preprocessor;
};

}

#endif // PP_ENVIRONMENT_H


