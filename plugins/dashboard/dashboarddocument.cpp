/* This file is part of KDevelop
  Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "dashboarddocument.h"

#include <QMimeDatabase>

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iuicontroller.h>
#include "dashboardview.h"

using namespace KDevelop;

DashboardDocument::DashboardDocument(KDevelop::IProject* project)
    : Sublime::UrlDocument(ICore::self()->uiController()->controller(), project->projectFileUrl()), IDocument(ICore::self())
    , m_project(project)
{}

void DashboardDocument::activate(Sublime::View* /*activeView*/, KParts::MainWindow* /*mainWindow*/)
{}

void DashboardDocument::setTextSelection(const KTextEditor::Range& /*range*/) {}
void DashboardDocument::setCursorPosition(const KTextEditor::Cursor& /*cursor*/) {}

KTextEditor::Cursor DashboardDocument::cursorPosition() const
{
    return KTextEditor::Cursor();
}

IDocument::DocumentState DashboardDocument::state() const
{
    return IDocument::Clean;
}

bool DashboardDocument::isActive() const
{
    return true;
}

bool DashboardDocument::close(KDevelop::IDocument::DocumentSaveMode /*mode*/)
{
    return true;
}

void DashboardDocument::reload()
{}

bool DashboardDocument::save(KDevelop::IDocument::DocumentSaveMode /*mode*/)
{
    return true;
}

KTextEditor::Document* DashboardDocument::textDocument() const
{
    return 0;
}

KParts::Part* DashboardDocument::partForView(QWidget* /*view*/) const
{
    return 0;
}

QMimeType DashboardDocument::mimeType() const
{
    return QMimeDatabase().mimeTypeForName("text/x-kdevelop");
}

Sublime::View* DashboardDocument::newView(Sublime::Document* doc)
{
    if( dynamic_cast<DashboardDocument*>( doc ) )
        return new DashboardView(m_project, doc);
    
    return 0;
}

QUrl DashboardDocument::url() const
{
    return m_project->projectFileUrl();
}
