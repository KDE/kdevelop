/*
 * KDevelop C++ Code Completion Support
 *
 * Copyright 2006-2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
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

#include <codecompletionworker.h>

#include "cppcodecompletionmodel.h"

class CppCodeCompletionWorker : public KDevelop::CodeCompletionWorker
{
  Q_OBJECT

  public:
    CppCodeCompletionWorker(CppCodeCompletionModel* parent);
    
    CppCodeCompletionModel* model() const;

  protected:
    virtual void computeCompletions(KDevelop::DUContextPointer context, const KTextEditor::Cursor& position, KTextEditor::View* view, const KTextEditor::Range& contextRange, const QString& contextText);

  private:
    void computeGroups(QList<KDevelop::CompletionTreeItemPointer> items, KSharedPtr<Cpp::CodeCompletionContext> completionContext);
};

#endif // KDEVCPPCODECOMPLETIONWORKER_H
