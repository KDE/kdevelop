/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "idocumentcontroller.h"

namespace KDevelop {

IDocumentController::IDocumentController(QObject *parent)
    :QObject(parent)
{
}

KDevelop::IDocument* IDocumentController::openDocument( const QUrl &url,
        const KTextEditor::Cursor& cursor,
        DocumentActivationParams activationParams,
        const QString& encoding)
{
    return openDocument(url, cursor.isValid() ? KTextEditor::Range(cursor, 0) : KTextEditor::Range::invalid(), activationParams, encoding);
}

}


