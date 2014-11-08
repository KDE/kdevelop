/*
 * This file is part of KDevelop
 * Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>
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

#include "standarddocumentationview.h"
#include "documentationfindwidget.h"
#include "debug.h"

using namespace KDevelop;

StandardDocumentationView::StandardDocumentationView(DocumentationFindWidget* findWidget, QWidget* parent)
    : QWebView (parent)
{
    findWidget->setEnabled(true);
    connect(findWidget, &DocumentationFindWidget::newSearch, this, &StandardDocumentationView::search);
}

void StandardDocumentationView::search ( const QString& text, DocumentationFindWidget::FindOptions options )
{
    //Highlighting has been commented because it doesn't let me jump around all occurrences
//     page()->findText(QString(), QWebPage::HighlightAllOccurrences);

    QWebPage::FindFlags ff=QWebPage::FindWrapsAroundDocument /*| QWebPage::HighlightAllOccurrences*/;
    if(options & DocumentationFindWidget::Previous)
        ff |= QWebPage::FindBackward;

    if(options & DocumentationFindWidget::MatchCase)
        ff |= QWebPage::FindCaseSensitively;

    page()->findText(text, ff);
}

void StandardDocumentationView::setDocumentation(const IDocumentation::Ptr& doc)
{
    if(m_doc)
        disconnect(m_doc.data());
    m_doc = doc;
    update();
    if(m_doc)
        connect(m_doc.data(), &IDocumentation::descriptionChanged, this, &StandardDocumentationView::update);
}

void StandardDocumentationView::update()
{
    if(m_doc)
        setHtml(m_doc->description());
    else
        qCDebug(DOCUMENTATION) << "calling StandardDocumentationView::update() on an unitialized view";
}
