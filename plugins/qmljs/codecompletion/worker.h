/*
    SPDX-FileCopyrightText: 2013 Sven Brauch <svenbrauch@googlemail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef QMLJS_WORKER_H
#define QMLJS_WORKER_H

#include <language/codecompletion/codecompletionworker.h>
#include <language/codecompletion/codecompletionmodel.h>

namespace QmlJS {

class CodeCompletionWorker : public KDevelop::CodeCompletionWorker
{
    Q_OBJECT
public:
    explicit CodeCompletionWorker(KDevelop::CodeCompletionModel* model);

protected:
    KDevelop::CodeCompletionContext* createCompletionContext(
        const KDevelop::DUContextPointer& context, const QString& contextText,
        const QString& followingText, const KDevelop::CursorInRevision& position) const override;
};

}

#endif // QMLJS_WORKER_H
