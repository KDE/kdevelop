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

#ifndef CLANGCODECOMPLETIONMODEL_H
#define CLANGCODECOMPLETIONMODEL_H

#include <language/codecompletion/codecompletionmodel.h>

#include "codecompletionexport.h"

#include <QMetaType>

class KDEVCLANGCODECOMPLETION_EXPORT ClangCodeCompletionModel : public KDevelop::CodeCompletionModel
{
    Q_OBJECT

public:
    explicit ClangCodeCompletionModel(QObject* parent);
    virtual ~ClangCodeCompletionModel();

signals:
    void requestCompletion(const QUrl &url, const KTextEditor::Cursor& cursor, const QString& text);

protected:
    KDevelop::CodeCompletionWorker* createCompletionWorker() override;

    void completionInvokedInternal(KTextEditor::View* view, const KTextEditor::Range& range,
                                   InvocationType invocationType, const QUrl &url) override;
};

Q_DECLARE_METATYPE(KTextEditor::Cursor)

#endif // CLANGCODECOMPLETIONMODEL_H
