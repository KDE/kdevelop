/*
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_ABSTRACTINCLUDENAVIGATIONCONTEXT_H
#define KDEVPLATFORM_ABSTRACTINCLUDENAVIGATIONCONTEXT_H

#include "abstractnavigationcontext.h"
#include "../../util/includeitem.h"
#include "../../duchain/parsingenvironment.h"
#include <language/languageexport.h>

namespace KDevelop {

/**
 * Abstract navigation context for file includes.
 * 
 * Example usage:
 * \code
 * namespace LANG {
 * class IncludeNavigationContext : public AbstractIncludeNavigationContext
 * {
 * public:
 *   IncludeNavigationContext(const IncludeItem& item, TopDuContextPointer topContext)
 *    : AbstractIncludeNavigationContext(item, topContext, KDevelop::LANGParsingEnvironment) {}
 * protected:
 *   virtual void getFileInfo(KDevelop::TopDUContext* duchain)
 *   {
 *   // write language dependent stuff via modifyHtml()
 *   }
 * };
 * }
 * \endcode
 */
class KDEVPLATFORMLANGUAGE_EXPORT AbstractIncludeNavigationContext : public AbstractNavigationContext {
public:
  AbstractIncludeNavigationContext(const IncludeItem& item, TopDUContextPointer topContext, 
                                   const ParsingEnvironmentType& type);
  virtual QString html(bool shorten);
  virtual QString name() const;

protected:
  /// Overwrite this to add language dependent information for a given file.
  /// By default only "included by" and "includes"
  /// NOTE: You should always append a newline (<br />) if you write anything.
  virtual void getFileInfo(KDevelop::TopDUContext* duchain);
  
  ///Should return true if this declaration should be shown, and false if not
  ///The duchain is locked when this is called
  virtual bool filterDeclaration(Declaration* decl);

private:
  /// Only environments with this type will be considered
  ParsingEnvironmentType m_type;
  ///@param first must initially be true
  typedef QPair<int, uint> IdentifierPair;
  void addDeclarationsFromContext(KDevelop::DUContext* ctx, bool& first,
                                  QList<IdentifierPair>& addedDeclarations,
                                  const QString& indent = "" );
  IncludeItem m_item;
};

}

#endif // KDEVPLATFORM_ABSTRACTINCLUDENAVIGATIONCONTEXT_H
