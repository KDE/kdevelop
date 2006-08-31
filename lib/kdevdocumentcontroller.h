/* This document is part of the KDE project
Copyright (C) 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
Copyright (C) 2002 Bernd Gehrmann <bernd@kdevelop.org>
Copyright (C) 2003 Roberto Raggi <roberto@kdevelop.org>
Copyright (C) 2003 Hamish Rodda <rodda@kde.org>
Copyright (C) 2003 Harald Fernengel <harry@kdevelop.org>
Copyright (C) 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
Copyright (C) 2004 Alexander Dymo <adymo@kdevelop.org>
Copyright (C) 2005 Adam Treat <treat@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the document COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/
#ifndef KDEV_DOCUMENTCONTROLLER_H
#define KDEV_DOCUMENTCONTROLLER_H

#include <QObject>
#include "kdevcore.h"

#include <QMap>
#include <QWidget>
#include <QPointer>
#include <QDateTime>
#include <QHash>

#include <kurl.h>
#include <kparts/partmanager.h>
#include <ktexteditor/cursor.h>
#include <ktexteditor/modificationinterface.h>

#include "kdevdocument.h"

/**
@document kdevdocumentcontroller.h
KDevelop document controller interface.
*/

namespace KParts
{
class Part;
class Factory;
class PartManager;
class ReadOnlyPart;
class ReadWritePart;
}

namespace KTextEditor
{
class Document;
class Editor;
}

class QTabWidget;

class KMenu;
class KAction;
class KToolBarPopupAction;
class KRecentFilesAction;
class KDirWatch;

class Context;

/**
 * \short Interface to control open documents.
 * The document controller manages open documents in the IDE.
 * Open documents are usually editors, GUI designers, html documentation etc.
*/
class KDEVINTERFACES_EXPORT KDevDocumentController: public QObject, public KDevCoreInterface
{
    friend class KDevCore;
    Q_OBJECT
    Q_CLASSINFO( "D-Bus Interface", "org.kdevelop.DocumentController" )
public:
    /**Constructor.
    @param parent The parent object.*/
    KDevDocumentController( QObject *parent = 0 );
    virtual ~KDevDocumentController();

    /**Call this before a call to @ref editDocument to set the encoding of the
    document to be opened.
    @param encoding The encoding to open as.*/
    void setEncoding( const QString &encoding );

    /**Shows a read-only document in the documentation viewer.
    @param url The Url of the document to view.*/
    KDevDocument* showDocumentation( const KUrl &url, bool newWin );

    /**Finds the first document object corresponding to a given url.
    @param url The Url of the document.
    @return The corresponding document, or null if not found.*/
    KDevDocument* documentForUrl( const KUrl & url ) const;

    /**@return The list of open documents*/
    QList<KDevDocument*> openDocuments() const;

    /**Saves a single document.
    @param doc the document to save
    @param force if true, force save even if the file was not modified.
    @return false if it was cancelled by the user, true otherwise */
    bool saveDocument( KDevDocument* document, bool force = false );

    /**Saves a list of documents.
    @param list The list of Urls to save.
    @return false if it was cancelled by the user, true otherwise */
    bool saveDocuments( const QList<KDevDocument*> &list );

    /**Reloads a document.
    * @param url The document to reload.*/
    void reloadDocument( KDevDocument* document );

    /**Reloads a list of documents.
    * @param list The documents to reload.*/
    void reloadDocuments( const QList<KDevDocument*> &list );

    /**Closes a document.
    * @param url The document to close.*/
    bool closeDocument( KDevDocument* document );

    /**Closes a list of documents.
    @param list The list of documents to close.*/
    bool closeDocuments( const QList<KDevDocument*> &list );

    /**Closes all other open documents.
    @param document The document not to close.*/
    bool closeAllOthers( const QList<KDevDocument*> &list );

    /**Activate this part.
    @param part The part to activate.*/
    void activateDocument( KDevDocument* document );

    /**Refers to the document currently active or focused.
    @return The Url of the active document.*/
    KDevDocument* activeDocument() const;

    /** Convenience function to proved the url of the currently active document, if one exists.
    @return The Url of the active document.*/
    KUrl activeDocumentUrl() const;

public Q_SLOTS:
    /**Opens a new or existing document.
    @param url The full Url of the document to open.
    @param range The location information, if applicable.
    @param activate Indicates whether to fully activate the document.*/
    Q_SCRIPTABLE KDevDocument* editDocument( const KUrl &url,
            const KTextEditor::Cursor& range = KTextEditor::Cursor::invalid(),
            bool activate = true );

    /**Saves all open documents.
     @return false if it was cancelled by the user, true otherwise */
    Q_SCRIPTABLE bool saveAllDocuments();

    /**Reloads all open documents.*/
    Q_SCRIPTABLE void reloadAllDocuments();

    /**Closes all open documents.*/
    Q_SCRIPTABLE bool closeAllDocuments();

    //FIXME figure out if these need to be public and/or use friend classes/document them
    void saveActiveDocument();
    void reloadActiveDocument();
    void closeActiveDocument();
    void closeAllExceptActiveDocument();

Q_SIGNALS:
    /**Emitted when the document is given focus or activated.*/
    void documentActivated( KDevDocument* document );

    /**Emitted when a document has been saved.*/
    void documentSaved( KDevDocument* document );

    /**Emitted when a document has been loaded.*/
    void documentLoaded( KDevDocument* document );

    /**Emitted when a document has been closed.*/
    void documentClosed( KDevDocument* document );

    /**Emitted when a document has been modified outside of KDevelop.*/
    void documentExternallyModified( KDevDocument* document );

    /**This is typically emitted when an editorpart does "save as"
    which will change the document's Url from 'old' to 'new'*/
    void documentUrlChanged( KDevDocument* document, const KUrl &oldUrl, const KUrl &newUrl );

    /**This is emitted when the document changes, either internally
    or on disc.*/
    void documentStateChanged( KDevDocument* document );

    //FIXME figure out if these need to be public and/or use friend classes/document them
    void openingDocument( const QString &document );

private Q_SLOTS:
    void contextMenu( KMenu *menu, const Context *context );

    void slotOpenDocument();
    void slotOpenRecent( const KUrl& );

    void slotBack();
    void slotForward();
    void slotBackAboutToShow();
    void slotForwardAboutToShow();
    void slotBackPopupActivated( int id );
    void slotForwardPopupActivated( int id );

    void slotSwitchTo();

    void slotUploadFinished();

    void updateMenuItems();

    void modifiedOnDisk( KTextEditor::Document * doc, bool isModified,
                         KTextEditor::ModificationInterface::ModifiedOnDiskReason reason );
    void newDocumentStatus( KTextEditor::Document * doc );

protected:
    virtual void loadSettings( bool projectIsLoaded );
    virtual void saveSettings( bool projectIsLoaded );
    virtual void initialize();
    virtual void cleanup();

private:
    bool querySaveDocuments();
    void setCursorPosition( KParts::Part *part,
                            const KTextEditor::Cursor& cursor );
    bool openAsDialog( const KUrl &url, KMimeType::Ptr mimeType );
    KDevDocument *addDocument( KParts::Part * part, bool setActive = true );
    void removeDocument( KDevDocument* document );
    void replaceDocument( KDevDocument* newDocument,
                          KDevDocument* oldDocument, bool setActive = true );
    void setActiveDocument( KDevDocument* document, QWidget *widget = 0L );

    KParts::Factory *findPartFactory( const QString &mimeType,
                                      const QString &partType,
                                      const QString &preferredName
                                      = QString::null );

    KDevDocument* integratePart( KParts::Part *part,
                                 bool activate = true );

    KUrl storedUrlForDocument( KDevDocument* ) const;
    void updateDocumentUrl( KDevDocument* );
    bool documentUrlHasChanged( KDevDocument* );

    KParts::ReadOnlyPart* activeReadOnly() const;
    KParts::ReadWritePart* activeReadWrite() const;
    KParts::ReadOnlyPart* readOnly( KParts::Part *part ) const;
    KParts::ReadWritePart* readWrite( KParts::Part *part ) const;
    KDevDocument* documentForPart( KParts::Part* part ) const;

    KAction *m_closeWindowAction;
    KAction *m_saveAllDocumentsAction;
    KAction *m_reloadAllDocumentsAction;
    KAction *m_closeAllWindowsAction;
    KAction *m_closeOtherWindowsAction;
    KAction *m_switchToAction;
    KRecentFilesAction *m_openRecentAction;
    QString m_presetEncoding;

    KToolBarPopupAction* m_backAction;
    KToolBarPopupAction* m_forwardAction;

    bool m_openNextAsText;

    QHash<KParts::ReadOnlyPart*, KDevDocument*> m_partHash;

    // used to note when a URL changes (a document changes url)
    QHash< KDevDocument*, KUrl > m_documentUrls;
    // used to map urls to open docs
    QHash< KUrl, KDevDocument* > m_url2Document;
    // used to fill the context menu
    KUrl::List m_selectedURLs;

    struct HistoryEntry
    {
        HistoryEntry()
        {}
        HistoryEntry( const KUrl & u, const KTextEditor::Cursor& cursor );

        KUrl url;
        KTextEditor::Cursor cursor;
        int id;
    };

    QList<HistoryEntry> m_backHistory;
    QList<HistoryEntry> m_forwardHistory;
    bool m_isJumping;

    HistoryEntry createHistoryEntry();
    void addHistoryEntry();
    void jumpTo( const HistoryEntry & );
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
