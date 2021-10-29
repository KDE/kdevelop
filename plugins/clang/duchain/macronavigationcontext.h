/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef MACRONAVIGATIONCONTEXT_H
#define MACRONAVIGATIONCONTEXT_H

#include "clangprivateexport.h"

#include "macrodefinition.h"

#include <language/editor/documentcursor.h>
#include <language/duchain/navigation/abstractnavigationcontext.h>

#include <QPointer>

class KDEVCLANGPRIVATE_EXPORT MacroNavigationContext : public KDevelop::AbstractNavigationContext
{
    Q_OBJECT
public:
    explicit MacroNavigationContext(const MacroDefinition::Ptr& macro,
                           const KDevelop::DocumentCursor& expansionLocation = KDevelop::DocumentCursor::invalid());
    ~MacroNavigationContext() override;

    QString html(bool shorten) override;
    QString name() const override;

private:
    QString retrievePreprocessedBody(const KDevelop::DocumentCursor& expansionLocation) const;

    const MacroDefinition::Ptr m_macro;
    QString m_body;
    QPointer<QWidget> m_widget;
};

#endif
