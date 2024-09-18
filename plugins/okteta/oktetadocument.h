/*
    SPDX-FileCopyrightText: 2010 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
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
    Q_INTERFACES(KDevelop::IDocument)

  public:
    OktetaDocument( const QUrl &url, ICore* core );

    ~OktetaDocument() override;

  public: // KDevelop::IDocument API
    KTextEditor::Cursor cursorPosition() const override;
    bool isActive() const override;
    QMimeType mimeType() const override;
    QIcon icon() const override;
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
    // Moc is too primitive to know about namespace aliases
    void onByteArrayDocumentLoaded( Kasten::AbstractDocument* document );

  private:
    OktetaPlugin* mPlugin;

    Kasten::ByteArrayDocument* mByteArrayDocument;
};


inline OktetaPlugin* OktetaDocument::plugin() const { return mPlugin; }
inline Kasten::ByteArrayDocument* OktetaDocument::byteArrayDocument() const { return mByteArrayDocument; }

}

#endif
