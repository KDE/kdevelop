/* This file is part of KDevelop
 *  Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_PLUGIN_DASHBOARDDOCUMENT_H
#define KDEVPLATFORM_PLUGIN_DASHBOARDDOCUMENT_H

#include <interfaces/idocument.h>
#include <sublime/urldocument.h>

class WelcomePageDocument : public Sublime::UrlDocument, public KDevelop::IDocument
{
    Q_OBJECT
public:
    WelcomePageDocument();

    void activate(Sublime::View* activeView, KParts::MainWindow* mainWindow) override;
    void setTextSelection(const KTextEditor::Range& range) override;
    void setCursorPosition(const KTextEditor::Cursor& cursor) override;
    KTextEditor::Cursor cursorPosition() const override;
    KDevelop::IDocument::DocumentState state() const override;
    bool isActive() const override;
    bool close(KDevelop::IDocument::DocumentSaveMode mode = Default) override;
    void reload() override;
    bool save(KDevelop::IDocument::DocumentSaveMode mode = Default) override;
    KTextEditor::Document* textDocument() const override;
    KParts::Part* partForView(QWidget* view) const override;
    QMimeType mimeType() const override;
    QUrl url() const override;

    static QUrl welcomePageUrl();
protected:
    Sublime::View* newView(Document* doc) override;
};

#endif // KDEVPLATFORM_PLUGIN_DASHBOARDDOCUMENT_H
