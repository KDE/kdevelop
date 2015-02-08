/* This file is part of KDevelop
 * Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "welcomepagedocument.h"

#include <QMimeDatabase>

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/isession.h>
#include "welcomepagedocument.h"
#include "welcomepageview.h"
#include <KLocalizedString>

using namespace KDevelop;

Q_GLOBAL_STATIC_WITH_ARGS(QUrl, s_welcomePageUrl, (QUrl("kdev:///awesome.kdevinternal")));

WelcomePageDocument::WelcomePageDocument()
    : Sublime::UrlDocument(ICore::self()->uiController()->controller(), *s_welcomePageUrl), IDocument(ICore::self())
{
    setTitle(i18n("Dashboard"));
}

void WelcomePageDocument::activate(Sublime::View* /*activeView*/, KParts::MainWindow* /*mainWindow*/)
{}

void WelcomePageDocument::setTextSelection(const KTextEditor::Range& /*range*/) {}
void WelcomePageDocument::setCursorPosition(const KTextEditor::Cursor& /*cursor*/) {}

KTextEditor::Cursor WelcomePageDocument::cursorPosition() const
{
    return KTextEditor::Cursor();
}

IDocument::DocumentState WelcomePageDocument::state() const
{
    return IDocument::Clean;
}

bool WelcomePageDocument::isActive() const
{
    return true;
}

bool WelcomePageDocument::close(KDevelop::IDocument::DocumentSaveMode /*mode*/)
{
    return true;
}

void WelcomePageDocument::reload()
{}

bool WelcomePageDocument::save(KDevelop::IDocument::DocumentSaveMode /*mode*/)
{
    return true;
}

KTextEditor::Document* WelcomePageDocument::textDocument() const
{
    return 0;
}

KParts::Part* WelcomePageDocument::partForView(QWidget* /*view*/) const
{
    return 0;
}

QMimeType WelcomePageDocument::mimeType() const
{
    return QMimeDatabase().mimeTypeForName("text/x-kdevelopinternal");
}

Sublime::View* WelcomePageDocument::newView(Sublime::Document* doc)
{
    if( dynamic_cast<WelcomePageDocument*>( doc ) )
        return new WelcomePageView(doc, Sublime::View::TakeOwnership);

    return 0;
}

QUrl WelcomePageDocument::url() const
{
    return *s_welcomePageUrl;
}

QUrl WelcomePageDocument::welcomePageUrl()
{
    return *s_welcomePageUrl;
}
