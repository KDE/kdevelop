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

#ifndef KDEVPLATFORM_RENAMEEASSISTANT_H
#define KDEVPLATFORM_RENAMEEASSISTANT_H

#include <language/assistant/staticassistant.h>
#include <language/duchain/identifier.h>
#include <language/duchain/indexedstring.h>
#include <language/editor/persistentmovingrange.h>
#include "renameaction.h"

namespace KTextEditor {
class View;
}

namespace KDevelop {
class ParseJob;

class KDEVPLATFORMLANGUAGE_EXPORT RenameAssistant : public StaticAssistant
{
    Q_OBJECT

public:
    RenameAssistant(ILanguageSupport* supportedLanguage);
    virtual ~RenameAssistant();

    virtual void textChanged(KTextEditor::View* view, const KTextEditor::Range& invocationRange, const QString& removedText = QString()) override;
    virtual bool isUseful() const override;

    virtual QString title() const override;

private:
    struct Private;
    QScopedPointer<Private> const d;

    Q_PRIVATE_SLOT(d, void reset());
};

}

#endif // KDEVPLATFORM_RENAMEEASSISTANT_H
