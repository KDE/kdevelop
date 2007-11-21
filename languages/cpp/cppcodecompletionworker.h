/*
 * KDevelop C++ Code Completion Support
 *
 * Copyright 2006-2007 Hamish Rodda <rodda@kde.org>
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

#ifndef KDEVCPPCODECOMPLETIONWORKER_H
#define KDEVCPPCODECOMPLETIONWORKER_H

#include <QThread>
#include <QList>

#include "cppcodecompletionmodel.h"

class CodeCompletionWorker : public QThread
{
  Q_OBJECT

  public:
    CodeCompletionWorker(CppCodeCompletionModel* parent);
    virtual ~CodeCompletionWorker();

    CppCodeCompletionModel* model() const;

    void abortCurrentCompletion();

  Q_SIGNALS:
    void foundDeclarations(QList<CppCodeCompletionModel::CompletionItem>, void* completionContext);

  protected:
    virtual void run();

  private Q_SLOTS:
    void computeCompletions(KDevelop::DUContextPointer context, const KTextEditor::Cursor& position, KTextEditor::View* view);

  private:
    KTextEditor::Cursor m_position;
    bool m_abort;
    QMutex* m_mutex;
};

#endif // KDEVCPPCODECOMPLETIONWORKER_H
