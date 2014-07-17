/* KDevelop CMake Support
 *
 * Copyright 2008 Aleix Pol <aleixpol@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef CMAKECODECOMPLETION_H
#define CMAKECODECOMPLETION_H

#include <ktexteditor/codecompletionmodel.h>
#include <language/duchain/duchainpointer.h>
#include <QStringList>
#include <language/duchain/declaration.h>

class CMakeDocumentation;
namespace KTextEditor { class Document; class Range; }

class CMakeCodeCompletionModel : public KTextEditor::CodeCompletionModel
{
    public:
        CMakeCodeCompletionModel(QObject *parent);

        void completionInvoked(KTextEditor::View* view, const KTextEditor::Range& range, InvocationType invocationType);
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
        void executeCompletionItem(KTextEditor::View* view, const KTextEditor::Range& word, const QModelIndex& index) const override;
    private:
        enum Type { Command, Variable, Macro, Path, Target };
        Type indexType(int row) const;
        static QStringList s_commands;
        QList< KDevelop::IndexedDeclaration > m_declarations;
        bool m_inside;
        QStringList m_paths;
};

#endif
