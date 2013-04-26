/***************************************************************************
 *   Copyright 2009 Aleix Pol <aleixpol@kde.org>                           *
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

#ifndef KDEVPLATFORM_PATCHDOCUMENT_H
#define KDEVPLATFORM_PATCHDOCUMENT_H

#include <sublime/urldocument.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include "partdocument.h"
#include <interfaces/ipatchdocument.h>

namespace KDevelop { class VcsDiff; }

class PatchDocument;

class PatchDocumentFactory : public KDevelop::IDocumentFactory
{
public:
    PatchDocumentFactory()
        : KDevelop::IDocumentFactory()
    {}

    KDevelop::IDocument* create( const KUrl& url, KDevelop::ICore* core);
};

namespace KParts { class ReadOnlyPart; }

class PatchDocument : public KDevelop::PartDocument, public KDevelop::IPatchDocument
{
    Q_OBJECT
    public:
        PatchDocument(const KUrl& url, KDevelop::ICore* core);
        virtual ~PatchDocument() {}
        
        QWidget* createViewWidget(QWidget* parent = 0);
//         KTextEditor::Cursor cursorPosition() const { return KTextEditor::Cursor(); }
//         void activate(Sublime::View* activeView, KParts::MainWindow* mainWindow) {}
//         bool close(DocumentSaveMode mode = Default) { return true; }
//         bool isActive() const { return true; }
//         KMimeType::Ptr mimeType() const;
//         void reload() {}
//         bool save(DocumentSaveMode mode = Default) { return true; }
//         void setCursorPosition(const KTextEditor::Cursor& cursor) {}
//         void setTextSelection(const KTextEditor::Range& range) {}
//         DocumentState state() const { return Clean; }
//         KTextEditor::Document* textDocument() const { return 0; }
//         KUrl url() const;
//         bool closeDocument();
        void setDiff(const QString& from, const KUrl& to);
};

#endif // KDEVPLATFORM_PATCHDOCUMENT_H
