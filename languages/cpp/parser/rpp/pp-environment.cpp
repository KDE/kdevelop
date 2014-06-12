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

#include <language/duchain/indexedstring.h> //Needs to be up here, so qHash(IndexedString) is found

#include "pp-environment.h"

#include <kdebug.h>

#include "pp-macro.h"
#include "pp-engine.h"
#include "pp-location.h"
#include "macrorepository.h"

using namespace rpp;

Environment::Environment()
  : m_locationTable(new LocationTable)
{
}

Environment::~Environment()
{
  delete m_locationTable;
  foreach(pp_macro* macro, m_ownedMacros)
    delete macro;
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

void Environment::swapMacros( Environment* parentEnvironment ) {
  qSwap(m_environment, parentEnvironment->m_environment);

  qSwap(m_ownedMacros, parentEnvironment->m_ownedMacros);
}

void Environment::clearMacro(const KDevelop::IndexedString& name)
{
  m_environment.remove(name);
}

void Environment::setMacro(pp_macro* macro)
{
  if(!macro->isRepositoryMacro())
    m_ownedMacros.append(macro);

  m_environment.insert(macro->name, macro);
}

void Environment::insertMacro(pp_macro* macro)
{
  m_environment.insert(macro->name, macro);
}

const Environment::EnvironmentMap& Environment::environment() const {
  return m_environment;
}

pp_macro* Environment::retrieveStoredMacro(const KDevelop::IndexedString& name) const
{
  return m_environment.value(name, nullptr);
}

pp_macro* Environment::retrieveMacro(const KDevelop::IndexedString& name, bool /*isImportant*/) const
{
  return retrieveStoredMacro(name);
}

QList<pp_macro*> Environment::allMacros() const
{
  return m_environment.values();
}
