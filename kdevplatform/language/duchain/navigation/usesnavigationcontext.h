/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_USESNAVIGATIONCONTEXT_H
#define KDEVPLATFORM_USESNAVIGATIONCONTEXT_H

#include "abstractnavigationwidget.h"
#include <language/languageexport.h>

namespace KDevelop {
class UsesWidget;
class KDEVPLATFORMLANGUAGE_EXPORT UsesNavigationContext
    : public AbstractNavigationContext
{
    Q_OBJECT

public:
    explicit UsesNavigationContext(KDevelop::IndexedDeclaration declaration,
                                   AbstractNavigationContext* previousContext = nullptr);

    ~UsesNavigationContext() override;

    QString name() const override;
    QWidget* widget() const override;
    QString html(bool shorten) override;

private:
    KDevelop::IndexedDeclaration m_declaration;
    UsesWidget* m_widget;
};
}

#endif
