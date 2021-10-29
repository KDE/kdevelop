/*
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef INCLUDEPATHCOMPLETIONCONTEXT_H
#define INCLUDEPATHCOMPLETIONCONTEXT_H

#include "duchain/parsesession.h"
#include "clangprivateexport.h"

#include <language/codecompletion/codecompletioncontext.h>
#include <language/util/includeitem.h>


struct KDEVCLANGPRIVATE_EXPORT IncludePathProperties
{
    // potentially already existing path to a directory
    QString prefixPath;
    // whether we look at a i.e. #include "local"
    // or a #include <global> include line
    bool local = false;
    // whether the line actually includes an #include
    bool valid = false;
    // start offset into @p text where to insert the new item
    int inputFrom = -1;
    // end offset into @p text where to insert the new item
    int inputTo = -1;

    static IncludePathProperties parseText(const QString& text, int rightBoundary = -1);
};

class KDEVCLANGPRIVATE_EXPORT IncludePathCompletionContext : public KDevelop::CodeCompletionContext
{
public:
    IncludePathCompletionContext(const KDevelop::DUContextPointer& context,
                                 const ParseSessionData::Ptr& sessionData,
                                 const QUrl& url,
                                 const KTextEditor::Cursor& position,
                                 const QString& text);

    QList<KDevelop::CompletionTreeItemPointer> completionItems(bool& abort, bool fullCompletion = true) override;

    ~IncludePathCompletionContext() override = default;

private:
    QVector<KDevelop::IncludeItem> m_includeItems;
};

#endif // INCLUDEPATHCOMPLETIONCONTEXT_H
