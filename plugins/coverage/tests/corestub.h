/* KDevelop xUnit plugin
 *    Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef TESTFAKES_CORESTUB_H
#define TESTFAKES_CORESTUB_H

#include "interfaces/icore.h"
#include <shell/uicontroller.h>
#include <shell/documentcontroller.h>
#include <KComponentData>

namespace KDevelop
{
class IDocumentController;
class ILanguageController;
class IPluginController;
class IProjectController;
class IRunController;
class IUiController;
class ISessionController;
}

namespace KParts
{
class KPartManager;
}

#include "interfaces/iuicontroller.h"

namespace TestStubs
{

class UiController : public KDevelop::UiController {
public:
    UiController( KDevelop::Core* c ) : KDevelop::UiController(c), m_activeMainWindow(0) {}
    virtual ~UiController() {}

    //enum SwitchMode {
    //    ThisWindow /**< indicates that the area switch should be in the this window */,
    //    NewWindow  /**< indicates that the area switch should be using a new window */
    //};

    virtual void switchToArea(const QString &areaName, SwitchMode switchMode) {}
    virtual void addToolView(const QString &name, KDevelop::IToolViewFactory *factory) {}
    virtual void removeToolView(KDevelop::IToolViewFactory *factory) {}
    virtual KParts::MainWindow *activeMainWindow() { return m_activeMainWindow; }
    virtual Sublime::Controller* controller() { return m_controller; }
    virtual void registerStatus(QObject*) {}

    KParts::MainWindow* m_activeMainWindow;
    Sublime::Controller* m_controller;
};

}

#include <interfaces/idocumentcontroller.h>

namespace TestStubs
{

class DocumentController : public KDevelop::DocumentController
{
//    Q_OBJECT
public:
#if 0
    enum DocumentActivation
    {
        DefaultMode = 0,            /**Activate document and create a view if no other flags passed.*/
        DoNotActivate = 1,          /**Don't activate the Document.*/
        DoNotCreateView = 2         /**Don't create and show the view for the Document.*/
    };
    Q_DECLARE_FLAGS(DocumentActivationParams, DocumentActivation)
#endif
    DocumentController(QObject *parent) : KDevelop::DocumentController(parent) {}
    virtual ~DocumentController() {}

    virtual void setEncoding( const QString &encoding ) {}
    virtual QString encoding() const { return ""; }

    virtual KDevelop::IDocument* documentForUrl( const KUrl & url ) const { return 0; }
    virtual QList<KDevelop::IDocument*> openDocuments() const { return QList<KDevelop::IDocument*>(); }
    virtual KDevelop::IDocument* activeDocument() const { return 0; }
    virtual void activateDocument( KDevelop::IDocument * document, const KTextEditor::Range& range = KTextEditor::Range::invalid() ) {}
    virtual void registerDocumentForMimetype( const QString&, KDevelop::IDocumentFactory* ) {}
    virtual bool saveAllDocuments(KDevelop::IDocument::DocumentSaveMode mode = KDevelop::IDocument::Default) { return true; }
    virtual KDevelop::IDocument* openDocumentFromText( const QString& data ) { return 0; }
    virtual void notifyDocumentClosed(KDevelop::IDocument* doc) {}

    // actually these are virtual slots, but so long as Qt does not cry we'r good.
    virtual KDevelop::IDocument* openDocument( const KUrl &url,
            const KTextEditor::Range& range = KTextEditor::Range::invalid(),
            DocumentActivationParams activationParams = 0 ) { return 0; }
    virtual void closeAllDocuments() {}

    void emitDocumentActivated(KDevelop::IDocument* doc) {
        emit documentActivated(doc);
    }
    void emitTextDocumentCreated(KDevelop::IDocument* doc) {
        emit textDocumentCreated(doc);
    }

#if 0
public Q_SLOTS:
    /**Opens a new or existing document.
    @param url The full Url of the document to open.
    @param range The location information, if applicable.
    @param activate Indicates whether to fully activate the document.*/
    KDevelop::IDocument* openDocument( const KUrl &url,
            const KTextEditor::Cursor& cursor,
            DocumentActivationParams activationParams = 0 );

    /**Opens a new or existing document.
    @param url The full Url of the document to open.
    @param range The range of text to select, if applicable.
    @param activate Indicates whether to fully activate the document.*/
    virtual KDevelop::IDocument* openDocument( const KUrl &url,
            const KTextEditor::Range& range = KTextEditor::Range::invalid(),
            DocumentActivationParams activationParams = 0 ) = 0;

    virtual void closeAllDocuments() = 0;

Q_SIGNALS:
    /**Emitted when the document has been activated.*/
    void documentActivated( KDevelop::IDocument* document );

    /**Emitted when a document has been saved.*/
    void documentSaved( KDevelop::IDocument* document );

    /**Emitted when a document has been loaded.
    Note, no views exist for the document at the time this signal is emitted.*/
    void documentLoaded( KDevelop::IDocument* document );

    /**Emitted when a text document has been loaded, and the text document created.
    Note, no views exist for the document at the time this signal is emitted.*/
    void textDocumentCreated( KDevelop::IDocument* document );

    /**Emitted when a document has been closed.*/
    void documentClosed( KDevelop::IDocument* document );

    /**This is emitted when the document state(the relationship
     * between the file in the editor and the file stored on disk) changes.*/
    void documentStateChanged( KDevelop::IDocument* document );

    /**This is emitted when the document content changed.*/
    void documentContentChanged( KDevelop::IDocument* document );

    /**Emitted when a document has been loaded, but before documentLoaded(..) is emitted.
     * this allows parts of kdevplatform to prepare data-structures that can be used by other parts
     * during documentLoaded(..).*/
    void documentLoadedPrepare( KDevelop::IDocument* document );

#endif
};

} // end namespace TestStubs

#endif // TESTFAKES_CORESTUB_H
