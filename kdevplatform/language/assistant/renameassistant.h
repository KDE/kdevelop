/*
    SPDX-FileCopyrightText: 2010 Olivier de Gaalon <olivier.jg@gmail.com>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_RENAMEASSISTANT_H
#define KDEVPLATFORM_RENAMEASSISTANT_H

#include <language/assistant/staticassistant.h>
#include <language/duchain/identifier.h>
#include "renameaction.h"

namespace KTextEditor {
class View;
}

namespace KDevelop {
class RenameAssistantPrivate;

class KDEVPLATFORMLANGUAGE_EXPORT RenameAssistant
    : public StaticAssistant
{
    Q_OBJECT

public:
    explicit RenameAssistant(ILanguageSupport* supportedLanguage);
    ~RenameAssistant() override;

    void textChanged(KTextEditor::Document* doc, const KTextEditor::Range& invocationRange,
                     const QString& removedText = QString()) override;
    bool isUseful() const override;
    KTextEditor::Range displayRange() const override;

    QString title() const override;

private:
    const QScopedPointer<class RenameAssistantPrivate> d_ptr;
    Q_DECLARE_PRIVATE(RenameAssistant)
};
}

#endif // KDEVPLATFORM_RENAMEASSISTANT_H
