/*
 * KDevelop C++ Code Completion Support
 *
 * Copyright (c) 2006 Hamish Rodda <rodda@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEVCPPCODECOMPLETIONMODEL_H
#define KDEVCPPCODECOMPLETIONMODEL_H

#include <ktexteditor/codecompletionmodel.h>

class DUContext;
class Declaration;

class CppCodeCompletionModel : public KTextEditor::CodeCompletionModel
{
  Q_OBJECT

  public:
    CppCodeCompletionModel(QObject* parent);
    virtual ~CppCodeCompletionModel();

    void setContext(DUContext* context, const KTextEditor::Cursor& position);

    virtual void completionInvoked(KTextEditor::View* view, const KTextEditor::Range& range, InvocationType invocationType);

    virtual QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;

  private:
    DUContext* m_context;

    QList<Declaration*> m_declarations;
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on
