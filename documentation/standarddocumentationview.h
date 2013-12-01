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

#ifndef KDEVPLATFORM_STANDARDDOCUMENTATIONVIEW_H
#define KDEVPLATFORM_STANDARDDOCUMENTATIONVIEW_H

#include <QWebView>
#include "documentationexport.h"
#include "documentationfindwidget.h"
#include <interfaces/idocumentation.h>

namespace KDevelop
{

/**
 * The standard documentation view, based on QWebView.
 */
class KDEVPLATFORMDOCUMENTATION_EXPORT StandardDocumentationView : public QWebView
{
    Q_OBJECT
public:
    explicit StandardDocumentationView(DocumentationFindWidget* findWidget, QWidget* parent = 0 );
    void setDocumentation(const IDocumentation::Ptr& doc);

public slots:
    /**
     * Search for @p text in the documentation view.
     */
    void search(const QString& text, KDevelop::DocumentationFindWidget::FindOptions options);

    /**
     * Updates the contents, in case it was initialized with a documentation instance,
     * doesn't change anything otherwise
     *
     * @sa setDocumentation(IDocumentation::Ptr)
     */
    void update();

private:
    IDocumentation::Ptr m_doc;
};

}
#endif // KDEVPLATFORM_STANDARDDOCUMENTATIONVIEW_H
