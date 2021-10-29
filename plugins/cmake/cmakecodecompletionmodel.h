/*
    SPDX-FileCopyrightText: 2008 Aleix Pol <aleixpol@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CMAKECODECOMPLETION_H
#define CMAKECODECOMPLETION_H

#include <language/duchain/duchainpointer.h>
#include <language/duchain/declaration.h>

#include <KTextEditor/CodeCompletionModel>

#include <QStringList>

class CMakeDocumentation;
namespace KTextEditor { class Document; class Range; }

class CMakeCodeCompletionModel : public KTextEditor::CodeCompletionModel
{
        Q_OBJECT
    public:
        explicit CMakeCodeCompletionModel(QObject *parent);

        void completionInvoked(KTextEditor::View* view, const KTextEditor::Range& range, InvocationType invocationType) override;
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const override;
        void executeCompletionItem(KTextEditor::View* view, const KTextEditor::Range& word, const QModelIndex& index) const override;
    private:
        enum Type { Command, Variable, Macro, Path, Target };
        Type indexType(int row) const;
        static QVector<QString> s_commands;
        QList< KDevelop::IndexedDeclaration > m_declarations;
        bool m_inside;
        QStringList m_paths;
};

#endif
