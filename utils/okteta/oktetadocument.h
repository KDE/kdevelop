/*
    This file is part of the KDevelop Okteta module, part of the KDE project.

    Copyright 2010 Friedrich W. H. Kossebau <kossebau@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OKTETADOCUMENT_H
#define OKTETADOCUMENT_H

// Plugin
#include "oktetaglobal.h"
// Kasten core
#include <kastencore.h>
// KDevPlatform
#include <sublime/urldocument.h>
#include <interfaces/idocument.h>
//Qt
#include <QtCore/QList>

namespace KASTEN_NAMESPACE {
class ByteArrayDocument;
class AbstractDocument;
}

namespace KDevelop
{
class ICore;
class OktetaPlugin;


class OktetaDocument : public Sublime::UrlDocument, public IDocument
{
    Q_OBJECT

  public:
    OktetaDocument( const KUrl& url, ICore* core );

    virtual ~OktetaDocument();

  public: // KDevelop::IDocument API
    virtual KTextEditor::Cursor cursorPosition() const;
    virtual bool isActive() const;
    virtual QMimeType mimeType() const;
    virtual KParts::Part* partForView( QWidget* widget ) const;
    virtual DocumentState state() const;
    virtual KTextEditor::Document* textDocument() const;
    virtual KUrl url() const;

    virtual void activate( Sublime::View* view, KParts::MainWindow* mainWindow );
    virtual bool close( IDocument::DocumentSaveMode = IDocument::Default );
    virtual void reload();
    virtual bool save( IDocument::DocumentSaveMode = IDocument::Default );
    virtual void setCursorPosition( const KTextEditor::Cursor& cursor );
    virtual void setTextSelection( const KTextEditor::Range& range );

  public: // Sublime::Document API
    virtual bool closeDocument(bool silent);

  public:
    OktetaPlugin* plugin() const;
    Kasten::ByteArrayDocument* byteArrayDocument() const;

  public:
    void setPlugin( OktetaPlugin* plugin );

  protected Q_SLOTS: // Sublime::Document API
    virtual Sublime::View* newView( Sublime::Document* document );

  protected Q_SLOTS:
    void onByteArrayDocumentChanged();
    // Moc is too primitive to know about namespace aliase
#if KASTEN_VERSION == 2
    void onByteArrayDocumentLoaded( Kasten2::AbstractDocument* document );
#elif KASTEN_VERSION == 1
    void onByteArrayDocumentLoaded( Kasten1::AbstractDocument* document );
#else
    void onByteArrayDocumentLoaded( Kasten::AbstractDocument* document );
#endif

  private:
    OktetaPlugin* mPlugin;

    Kasten::ByteArrayDocument* mByteArrayDocument;
};


inline OktetaPlugin* OktetaDocument::plugin() const { return mPlugin; }
inline Kasten::ByteArrayDocument* OktetaDocument::byteArrayDocument() const { return mByteArrayDocument; }

}

#endif
