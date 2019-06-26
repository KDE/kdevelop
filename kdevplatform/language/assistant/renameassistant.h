/*
   Copyright 2010 Olivier de Gaalon <olivier.jg@gmail.com>
   Copyright 2014 Kevin Funk <kfunk@kde.org>

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
