/*
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_ICONTEXTBROWSER_H
#define KDEVPLATFORM_ICONTEXTBROWSER_H

#include <language/languageexport.h>

#include <language/duchain/duchainpointer.h>

#include <QObject>

namespace KDevelop {
/**
 * @brief Interface to the context browser plugin.
 */
class KDEVPLATFORMLANGUAGE_EXPORT IContextBrowser
{
public:
    virtual ~IContextBrowser();

    /**
     * Show a widget with the uses of @p declaration.
     */
    virtual void showUses(const DeclarationPointer& declaration) = 0;
};
}

Q_DECLARE_INTERFACE(KDevelop::IContextBrowser, "org.kdevelop.IContextBrowser")

#endif // KDEVPLATFORM_ICONTEXTBROWSER_H
