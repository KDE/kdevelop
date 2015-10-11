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

#include "assistantcompletionmodel.h"

#include "staticassistantsmanager.h"

#include "interfaces/iassistant.h"
#include "interfaces/icore.h"
#include "interfaces/ilanguagecontroller.h"

namespace KDevelop {

AssistantCompletionModel::AssistantCompletionModel(QObject* parent)
    : KTextEditor::CodeCompletionModel(parent)
{
}

void AssistantCompletionModel::executeCompletionItem(KTextEditor::View*, const KTextEditor::Range&, const QModelIndex& index) const
{
    if (auto action = actionForIndex(index)) {
        action->execute();
    }
}

void AssistantCompletionModel::setActions(const QList<IAssistantAction::Ptr>& actions)
{
    if (m_actions == actions) {
        return;
    }

    beginResetModel();
    m_actions = actions;
    endResetModel();

    if (rowCount()) {
        emit hasCompletions();
    }
}

QVariant AssistantCompletionModel::data(const QModelIndex& index, const int role) const
{
    if (role == Qt::DisplayRole && index.column() == Name) {
        if (auto action = actionForIndex(index)) {
            return action->description();
        }
    } else if (role == ArgumentHintDepth) {
        return 1;
    }
    return {};
}

QModelIndex AssistantCompletionModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return {};

    return createIndex(row, column);
}

int AssistantCompletionModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_actions.size();
}

IAssistantAction* AssistantCompletionModel::actionForIndex(const QModelIndex& index) const
{
    if (index.row() < 0 || index.row() >= m_actions.size()) {
        return nullptr;
    }

    return m_actions.at(index.row()).data();
}

}
