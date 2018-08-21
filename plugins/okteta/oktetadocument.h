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

// Kasten core
#include <Kasten/KastenCore>
// KDevPlatform
#include <sublime/urldocument.h>
#include <interfaces/idocument.h>

namespace Kasten {
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
    OktetaDocument( const QUrl &url, ICore* core );

    ~OktetaDocument() override;

  public: // KDevelop::IDocument API
    KTextEditor::Cursor cursorPosition() const override;
    bool isActive() const override;
    QMimeType mimeType() const override;
    KParts::Part* partForView( QWidget* widget ) const override;
    DocumentState state() const override;
    KTextEditor::Document* textDocument() const override;
    QUrl url() const override;

    void activate( Sublime::View* view, KParts::MainWindow* mainWindow ) override;
    bool close( IDocument::DocumentSaveMode = IDocument::Default ) override;
    void reload() override;
    bool save( IDocument::DocumentSaveMode = IDocument::Default ) override;
    void setCursorPosition( const KTextEditor::Cursor& cursor ) override;
    void setTextSelection( const KTextEditor::Range& range ) override;

  public: // Sublime::Document API
    bool closeDocument(bool silent) override;

  public:
    OktetaPlugin* plugin() const;
    Kasten::ByteArrayDocument* byteArrayDocument() const;

  public:
    void setPlugin( OktetaPlugin* plugin );

  protected Q_SLOTS: // Sublime::Document API
    Sublime::View* newView( Sublime::Document* document ) override;

  protected Q_SLOTS:
    void onByteArrayDocumentChanged();
    // Moc is too primitive to know about namespace aliase
    void onByteArrayDocumentLoaded( Kasten::AbstractDocument* document );

  private:
    OktetaPlugin* mPlugin;

    Kasten::ByteArrayDocument* mByteArrayDocument;
};


inline OktetaPlugin* OktetaDocument::plugin() const { return mPlugin; }
inline Kasten::ByteArrayDocument* OktetaDocument::byteArrayDocument() const { return mByteArrayDocument; }

}

#endif
