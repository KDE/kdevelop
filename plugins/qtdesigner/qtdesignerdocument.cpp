/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "qtdesignerdocument.h"

#include "qtdesignerplugin.h"
#include "icore.h"
#include "iuicontroller.h"
#include <QtGui/QWidget>

QtDesignerDocument::QtDesignerDocument( const KUrl& url , KDevelop::ICore* core )
    : Sublime::UrlDocument(core->uiController()->controller(), url), KDevelop::IDocument(core), m_url(url)
{
}

KUrl QtDesignerDocument::url() const
{
    return m_url;
}

KSharedPtr<KMimeType> QtDesignerDocument::mimeType() const
{
    return KMimeType::mimeType("application/x-designer");
}

KParts::Part* QtDesignerDocument::partForView(QWidget*) const
{
    return 0;
}

KTextEditor::Document* QtDesignerDocument::textDocument() const
{
    return 0;
}

bool QtDesignerDocument::save(DocumentSaveMode)
{
    return true;
}

void QtDesignerDocument::reload()
{
}

void QtDesignerDocument::close()
{
}

bool QtDesignerDocument::isActive() const
{
    return true;
}

KDevelop::IDocument::DocumentState QtDesignerDocument::state() const
{
    return KDevelop::IDocument::Clean;
}

void QtDesignerDocument::setCursorPosition(const KTextEditor::Cursor&)
{
    return;
}

void QtDesignerDocument::activate(Sublime::View* view)
{
    Q_UNUSED(view)
    notifyActivated();
}

void QtDesignerDocument::setDesignerPlugin(QtDesignerPlugin* plugin)
{
    m_designerPlugin = plugin;
}

QWidget *QtDesignerDocument::createViewWidget(QWidget *parent)
{
    kDebug(9000) << "CREATING NEW UI WIDGET" << endl;
    return new QWidget(parent);
}


//kate: space-indent on;indent-width 4;replace-tabs on;auto-insert-doxygen on;indent-mode cstyle;
