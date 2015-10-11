/*
   Copyright 2015 Olivier de Gaalon <olivier.jg@gmail.com>

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

#ifndef ASSISTANTCOMPLETIONMODEL_H
#define ASSISTANTCOMPLETIONMODEL_H

#include <KTextEditor/CodeCompletionModel>

#include <QExplicitlySharedDataPointer>

namespace KDevelop {

class IAssistantAction;

class AssistantCompletionModel : public KTextEditor::CodeCompletionModel
{
    Q_OBJECT
public:
    AssistantCompletionModel(QObject* parent = {});

    void setActions(const QList<QExplicitlySharedDataPointer<IAssistantAction>> &actions);

    void executeCompletionItem(KTextEditor::View* view, const KTextEditor::Range& word, const QModelIndex& index) const override;

    QVariant data(const QModelIndex& index, const int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

signals:
    void hasCompletions();

private:
    IAssistantAction *actionForIndex(const QModelIndex& index) const;

    QList<QExplicitlySharedDataPointer<IAssistantAction>> m_actions;
};

}

#endif // ASSISTANTCOMPLETIONMODEL_H
