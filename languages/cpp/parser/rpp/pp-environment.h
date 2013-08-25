/*
  Copyright 2006 Hamish Rodda <rodda@kde.org>
  Copyright 2008-2009 David Nolden <david.nolden.kdevelop@art-master.de>

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
// #include "pp-macro.h"

namespace KDevelop {
  class IndexedString;
}

namespace rpp {

class pp_macro;
class LocationTable;

class KDEVCPPRPP_EXPORT Environment
{
public:
  typedef QHash<KDevelop::IndexedString, pp_macro*> EnvironmentMap;

  Environment();
  virtual ~Environment();

  void clearMacro(const KDevelop::IndexedString& name);

  //The macro will be owned by the environment object
  //Note: Undef-macros are allowed too
  virtual void setMacro(pp_macro* macro);

  //Inserts a macro that will not be explicitly owned by the Environment,
  //without notifying subclasses etc.
  void insertMacro(pp_macro* macro);
  
  virtual pp_macro* retrieveMacro(const KDevelop::IndexedString& name, bool isImportant) const;
  
  //Returns macros that are really stored locally(retrieveMacro may be overridden to perform more complex actions)
  pp_macro* retrieveStoredMacro(const KDevelop::IndexedString& name) const;
  
  //Returns all currently visible macros
  QList<pp_macro*> allMacros() const;

  //Swap the macros with the given environment, includign ownership
  virtual void swapMacros( Environment* parentEnvironment );

  //Faster access then allMacros(..), because nothing is copied
  const EnvironmentMap& environment() const; //krazy:exclude=constref

  LocationTable* locationTable() const;
  LocationTable* takeLocationTable();

private:
  EnvironmentMap m_environment;

  QVector<pp_macro*> m_ownedMacros;
  LocationTable* m_locationTable;
};

}

#endif // PP_ENVIRONMENT_H


