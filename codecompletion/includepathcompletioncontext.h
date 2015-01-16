/*
 * This file is part of KDevelop
 * Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>
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

#ifndef INCLUDEPATHCOMPLETIONCONTEXT_H
#define INCLUDEPATHCOMPLETIONCONTEXT_H

#include "duchain/parsesession.h"

#include <language/codecompletion/codecompletioncontext.h>
#include <language/util/includeitem.h>

class IncludePathCompletionContext : public KDevelop::CodeCompletionContext
{
public:
    IncludePathCompletionContext(const KDevelop::DUContextPointer& context,
                                 const ParseSessionData::Ptr& sessionData,
                                 const QUrl& url,
                                 const KTextEditor::Cursor& position,
                                 const QString& text);

    virtual QList<KDevelop::CompletionTreeItemPointer> completionItems(bool& abort, bool fullCompletion = true) override;

    virtual ~IncludePathCompletionContext() = default;

private:
    QList<KDevelop::IncludeItem> m_includeItems;

};

#endif // INCLUDEPATHCOMPLETIONCONTEXT_H
