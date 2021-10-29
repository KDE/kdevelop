/*
    SPDX-FileCopyrightText: 2014 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef NAVIGATIONWIDGET_H
#define NAVIGATIONWIDGET_H

#include "clangprivateexport.h"

#include "macrodefinition.h"

#include <language/duchain/navigation/abstractnavigationwidget.h>

namespace KDevelop
{
class DocumentCursor;
class IncludeItem;
}

class KDEVCLANGPRIVATE_EXPORT ClangNavigationWidget : public KDevelop::AbstractNavigationWidget
{
    Q_OBJECT
public:
    explicit ClangNavigationWidget(const KDevelop::DeclarationPointer& declaration,
                          KDevelop::AbstractNavigationWidget::DisplayHints hints = KDevelop::AbstractNavigationWidget::NoHints);
    ClangNavigationWidget(const MacroDefinition::Ptr& macro, const KDevelop::DocumentCursor& expansionLocation,
                          KDevelop::AbstractNavigationWidget::DisplayHints hints = KDevelop::AbstractNavigationWidget::NoHints);
    ClangNavigationWidget(const KDevelop::IncludeItem& includeItem, const KDevelop::TopDUContextPointer& topContext,
                          KDevelop::AbstractNavigationWidget::DisplayHints hints = KDevelop::AbstractNavigationWidget::NoHints);
    ~ClangNavigationWidget() override = default;

    /// Used by @see AbstractIncludeFileCompletionItem
    static QString shortDescription(const KDevelop::IncludeItem& includeItem);
};

#endif // NAVIGATIONWIDGET_H
