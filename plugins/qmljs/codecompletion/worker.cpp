/*
    SPDX-FileCopyrightText: 2013 Sven Brauch <svenbrauch@googlemail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "worker.h"
#include "context.h"

using namespace KDevelop;

namespace QmlJS {

CodeCompletionWorker::CodeCompletionWorker(CodeCompletionModel* model)
    : KDevelop::CodeCompletionWorker(model)
{

}

KDevelop::CodeCompletionContext* CodeCompletionWorker::createCompletionContext(const DUContextPointer& context,
          const QString& contextText, const QString& /*followingText*/, const CursorInRevision& position) const
{
    return new QmlJS::CodeCompletionContext(context, contextText, position);
}

}
