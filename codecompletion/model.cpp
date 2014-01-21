/*
 * This file is part of KDevelop
 * Copyright 2014 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "model.h"
#include "context.h"

#include <language/codecompletion/codecompletionworker.h>

using namespace KDevelop;

namespace {
class ClangCodeCompletionWorker : public CodeCompletionWorker
{
    Q_OBJECT
public:
    ClangCodeCompletionWorker(CodeCompletionModel* model)
        : CodeCompletionWorker(model)
    {}
    virtual ~ClangCodeCompletionWorker()
    {}

    CodeCompletionContext* createCompletionContext(DUContextPointer context, const QString& contextText,
                                                   const QString& /*followingText*/, const CursorInRevision& position) const override
    {
        return new ClangCodeCompletionContext(context, contextText, position);
    }
};
}

ClangCodeCompletionModel::ClangCodeCompletionModel(QObject* parent)
    : CodeCompletionModel(parent)
{
}

ClangCodeCompletionModel::~ClangCodeCompletionModel()
{

}

CodeCompletionWorker* ClangCodeCompletionModel::createCompletionWorker()
{
    return new ClangCodeCompletionWorker(this);
}

#include "model.moc"
#include "moc_model.cpp"
