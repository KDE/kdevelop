/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
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
class KDEVPLATFORMLANGUAGE_EXPORT AbstractIncludeNavigationContext
    : public AbstractNavigationContext
{
    Q_OBJECT

public:
    AbstractIncludeNavigationContext(const IncludeItem& item, const TopDUContextPointer& topContext,
                                     const ParsingEnvironmentType& type);
    QString html(bool shorten) override;
    QString name() const override;

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
    using IdentifierPair = QPair<int, uint>;
    ///@param first must initially be true
    void addDeclarationsFromContext(KDevelop::DUContext* ctx, bool& first,
                                    QVector<IdentifierPair>& addedDeclarations,
                                    const QString& indent = {});
    IncludeItem m_item;
};
}

#endif // KDEVPLATFORM_ABSTRACTINCLUDENAVIGATIONCONTEXT_H
