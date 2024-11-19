/*
    SPDX-FileCopyrightText: 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2002 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 2003 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2003-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2003 Harald Fernengel <harry@kdevelop.org>
    SPDX-FileCopyrightText: 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
    SPDX-FileCopyrightText: 2005 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2004-2007 Alexander Dymo <adymo@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "documentcontroller.h"

#include <QApplication>
#include <QDBusConnection>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QRegularExpression>
#include <QPointer>

#include <KActionCollection>
#include <KEncodingFileDialog>
#include <KIO/StatJob>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>
#include <KProtocolInfo>
#include <KRecentFilesAction>
#include <KTextEditor/Document>
#include <KTextEditor/View>

#include <sublime/area.h>
#include <sublime/message.h>
#include <sublime/view.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/ibuddydocumentfinder.h>
#include <interfaces/iproject.h>
#include <interfaces/iselectioncontroller.h>
#include <interfaces/context.h>
#include <project/projectmodel.h>
#include <util/scopeddialog.h>
#include <util/path.h>

#include "core.h"
#include "mainwindow.h"
#include "textdocument.h"
#include "uicontroller.h"
#include "partcontroller.h"
#include "savedialog.h"
#include "debug.h"

#include <vcs/interfaces/ibasicversioncontrol.h>
#include <vcs/vcspluginhelper.h>

#include <algorithm>

#define EMPTY_DOCUMENT_URL i18n("Untitled")

using namespace KDevelop;


class KDevelop::DocumentControllerPrivate
{
public:
    struct OpenFileResult
    {
        QList<QUrl> urls;
        QString encoding;
    };

    explicit DocumentControllerPrivate(DocumentController* c)
        : controller(c)
        , fileOpenRecent(nullptr)
    {
    }

    ~DocumentControllerPrivate() = default;

    // used to map urls to open docs
    QHash< QUrl, IDocument* > documents;
    bool shuttingDown = false;

    QHash< QString, IDocumentFactory* > factories;

    struct HistoryEntry
    {
        HistoryEntry() {}
        HistoryEntry( const QUrl & u, const KTextEditor::Cursor& cursor );

        QUrl url;
        KTextEditor::Cursor cursor;
        int id;
    };

    void removeDocument(Sublime::Document *doc)
    {
        const QList<QUrl> urlsForDoc = documents.keys(qobject_cast<KDevelop::IDocument*>(doc));
        for (const QUrl& url : urlsForDoc) {
            qCDebug(SHELL) << "destroying document" << doc;
            documents.remove(url);
        }
    }

    OpenFileResult showOpenFile() const
    {
        QUrl dir;
        if ( controller->activeDocument() ) {
            dir = controller->activeDocument()->url().adjusted(QUrl::RemoveFilename);
        } else  {
            const auto cfg = KSharedConfig::openConfig()->group(QStringLiteral("Open File"));
            dir = cfg.readEntry( "Last Open File Directory", Core::self()->projectController()->projectsBaseDirectory() );
        }

        const auto caption = i18nc("@title:window", "Open File");
        const auto filter = i18n("*|Text File\n");
        auto parent = Core::self()->uiControllerInternal()->defaultMainWindow();

        // use special dialogs in a KDE session, native dialogs elsewhere
        if (qEnvironmentVariableIsSet("KDE_FULL_SESSION")) {
            const auto result = KEncodingFileDialog::getOpenUrlsAndEncoding(QString(), dir,
                filter, parent, caption);
            return {result.URLs, result.encoding};
        }

        // note: can't just filter on text files using the native dialog, just display all files
        // see https://phabricator.kde.org/D622#11679
        const auto urls = QFileDialog::getOpenFileUrls(parent, caption, dir);
        return {urls, QString()};
    }

    void chooseDocument()
    {
        const auto res = showOpenFile();
        if( !res.urls.isEmpty() ) {
            QString encoding = res.encoding;
            for (const QUrl& u : res.urls) {
                openDocumentInternal(u, QString(), KTextEditor::Range::invalid(), encoding  );
            }
        }

    }

    void changeDocumentUrl(KDevelop::IDocument* document, const QUrl& previousUrl)
    {
        const auto it = documents.constFind(previousUrl);
        if (it == documents.cend()) {
            qCWarning(SHELL) << "a renamed document is not registered:" << document << previousUrl.toString()
                             << document->url().toString();
            return;
        }
        Q_ASSERT(it.value() == document);

        const auto documentIt = documents.constFind(document->url());
        if (documentIt != documents.constEnd()) {
            // Weird situation (saving as a file that is already open)
            IDocument* origDoc = *documentIt;
            Q_ASSERT_X(origDoc != document, Q_FUNC_INFO, "Duplicate documentUrlChanged signal emission?");
            if (origDoc->state() & IDocument::Modified) {
                // given that the file has been saved, close the saved file as the other instance will become conflicted on disk
                document->close(); // this closing erases the iterator `it`
                controller->activateDocument( origDoc );
                return;
            }
            // Otherwise close the original document, but first erase the iterator `it`,
            // because the closing erases documentIt, which can invalidate `it`.
            documents.erase(it);
            origDoc->close();
        } else {
            documents.erase(it); // erase the previous-URL entry
        }

        documents.insert(document->url(), document);

        if (!controller->isEmptyDocumentUrl(document->url()))
        {
            fileOpenRecent->addUrl(document->url());
        }
    }

    KDevelop::IDocument* findBuddyDocument(const QUrl &url, IBuddyDocumentFinder* finder)
    {
        const QList<KDevelop::IDocument*> allDocs = controller->openDocuments();
        for (KDevelop::IDocument* doc : allDocs) {
            if(finder->areBuddies(url, doc->url())) {
                return doc;
            }
        }
        return nullptr;
    }

    static bool fileExists(const QUrl& url)
    {
        if (url.isLocalFile()) {
            return QFile::exists(url.toLocalFile());
        } else {
            auto job = KIO::stat(url, KIO::StatJob::SourceSide, KIO::StatNoDetails, KIO::HideProgressInfo);
            KJobWidgets::setWindow(job, ICore::self()->uiController()->activeMainWindow());
            return job->exec();
        }
    };

    IDocument* openDocumentInternal( const QUrl & inputUrl, const QString& prefName = QString(),
        const KTextEditor::Range& range = KTextEditor::Range::invalid(), const QString& encoding = QString(),
        DocumentController::DocumentActivationParams activationParams = {},
        IDocument* buddy = nullptr)
    {
        Q_ASSERT(!inputUrl.isRelative());
        Q_ASSERT(!inputUrl.fileName().isEmpty() || !inputUrl.isLocalFile());
        QString _encoding = encoding;

        QUrl url = inputUrl;

        if ( url.isEmpty() && (!activationParams.testFlag(IDocumentController::DoNotCreateView)) )
        {
            const auto res = showOpenFile();
            if( !res.urls.isEmpty() )
                url = res.urls.first();
            _encoding = res.encoding;
            if ( url.isEmpty() )
                //still no url
                return nullptr;
        }

        KSharedConfig::openConfig()->group(QStringLiteral("Open File")).writeEntry("Last Open File Directory", url.adjusted(QUrl::RemoveFilename));

        // clean it and resolve possible symlink
        url = url.adjusted( QUrl::NormalizePathSegments );
        if ( url.isLocalFile() )
        {
            QString path = QFileInfo( url.toLocalFile() ).canonicalFilePath();
            if ( !path.isEmpty() )
                url = QUrl::fromLocalFile( path );
        }

        //get a part document
        IDocument* doc = documents.value(url);
        if (!doc)
        {
            QMimeType mimeType;

            if (DocumentController::isEmptyDocumentUrl(url))
            {
                mimeType = QMimeDatabase().mimeTypeForName(QStringLiteral("text/plain"));
            }
            else if (!url.isValid())
            {
                // Exit if the url is invalid (should not happen)
                // If the url is valid and the file does not already exist,
                // kate creates the file and gives a message saying so
                qCDebug(SHELL) << "invalid URL:" << url.url();
                return nullptr;
            }
            else if (KProtocolInfo::isKnownProtocol(url.scheme()) && !fileExists(url))
            {
                //Don't create a new file if we are not in the code mode.
                if (ICore::self()->uiController()->activeArea()->objectName() != QLatin1String("code")) {
                    return nullptr;
                }
                // enfore text mime type in order to create a kate part editor which then can be used to create the file
                // otherwise we could end up opening e.g. okteta which then crashes, see: https://bugs.kde.org/id=326434
                mimeType = QMimeDatabase().mimeTypeForName(QStringLiteral("text/plain"));
            }
            else
            {
                mimeType = QMimeDatabase().mimeTypeForUrl(url);

                if(!url.isLocalFile() && mimeType.isDefault())
                {
                    // fall back to text/plain, for remote files without extension, i.e. COPYING, LICENSE, ...
                    // using a synchronous KIO::MimetypeJob is hazardous and may lead to repeated calls to
                    // this function without it having returned in the first place
                    // and this function is *not* reentrant, see assert below:
                    // Q_ASSERT(!documents.contains(url) || documents[url]==doc);
                    mimeType = QMimeDatabase().mimeTypeForName(QStringLiteral("text/plain"));
                }
            }

            // is the URL pointing to a directory?
            if (mimeType.inherits(QStringLiteral("inode/directory")))
            {
                qCDebug(SHELL) << "cannot open directory:" << url.url();
                return nullptr;
            }

            if( prefName.isEmpty() )
            {
                // Try to find a plugin that handles this mimetype
                QVariantMap constraints;
                constraints.insert(QStringLiteral("X-KDevelop-SupportedMimeTypes"), mimeType.name());
                Core::self()->pluginController()->pluginForExtension(QString(), QString(), constraints);
            }

            if( IDocumentFactory* factory = factories.value(mimeType.name()))
            {
                doc = factory->create(url, Core::self());
            }

            if(!doc) {
                if( !prefName.isEmpty() )
                {
                    doc = new PartDocument(url, Core::self(), prefName);
                } else  if ( Core::self()->partControllerInternal()->isTextType(mimeType))
                {
                    doc = new TextDocument(url, Core::self(), _encoding);
                } else if( Core::self()->partControllerInternal()->canCreatePart(url) )
                {
                    doc = new PartDocument(url, Core::self());
                } else
                {
                    int openAsText = KMessageBox::questionTwoActions(
                        nullptr,
                        i18n("KDevelop could not find the editor for file '%1' of type %2.\nDo you want to open it as "
                             "plain text?",
                             url.fileName(), mimeType.name()),
                        i18nc("@title:window", "Could Not Find Editor"),
                        KGuiItem(i18nc("@action:button", "Open as Plain Text"), QStringLiteral("text-plaim")),
                        KGuiItem(i18nc("@action:button", "Do Not Open"), QStringLiteral("dialog-cancel")),
                        QStringLiteral("AskOpenWithTextEditor"));
                    if (openAsText == KMessageBox::PrimaryAction)
                        doc = new TextDocument(url, Core::self(), _encoding);
                    else
                        return nullptr;
                }
            }
        }

        // The url in the document must equal the current url, else the housekeeping will get broken
        Q_ASSERT(!doc || doc->url() == url);

        if(doc && openDocumentInternal(doc, range, activationParams, buddy))
            return doc;
        else
            return nullptr;

    }

    bool openDocumentInternal(IDocument* doc,
                                const KTextEditor::Range& range,
                                DocumentController::DocumentActivationParams activationParams,
                                IDocument* buddy = nullptr)
    {
        IDocument* previousActiveDocument = controller->activeDocument();
        KTextEditor::View* previousActiveTextView = ICore::self()->documentController()->activeTextDocumentView();
        KTextEditor::Cursor previousActivePosition;
        if(previousActiveTextView)
            previousActivePosition = previousActiveTextView->cursorPosition();

        QUrl url=doc->url();
        UiController *uiController = Core::self()->uiControllerInternal();
        Sublime::Area *area = uiController->activeArea();

        //We can't have the same url in many documents
        //so we check it's already the same if it exists
        //contains=>it's the same
        Q_ASSERT(!documents.contains(url) || documents[url]==doc);

        auto *sdoc = dynamic_cast<Sublime::Document*>(doc);
        if( !sdoc )
        {
            documents.remove(url);
            delete doc;
            return false;
        }

        //We check if it was already opened before
        const bool wasClosed = !documents.contains(url);
        if (wasClosed) {
            documents[url]=doc;

            // react on document deletion - we need to clean up controller structures
            QObject::connect(sdoc, &Sublime::Document::aboutToDelete, controller,
                             &DocumentController::notifyDocumentClosed);
        }

        if (!activationParams.testFlag(IDocumentController::DoNotCreateView))
        {
            //find a view if there's one already opened in this area
            Sublime::AreaIndex* activeViewIdx = area->indexOf(uiController->activeSublimeWindow()->activeView());
            const auto& views = sdoc->views();
            auto it = std::find_if(views.begin(), views.end(), [&](Sublime::View* view) {
                Sublime::AreaIndex* areaIdx = area->indexOf(view);
                return (areaIdx && areaIdx == activeViewIdx);
            });
            Sublime::View* partView = (it != views.end()) ? *it : nullptr;
            bool addView = false;
            if (!partView)
            {
                //no view currently shown for this url
                partView = sdoc->createView();
                addView = true;
            }

            if(addView) {
                // This code is never executed when restoring session on startup,
                // only when opening a file manually

                Sublime::View* buddyView = nullptr;
                bool placeAfterBuddy = true;
                if(Core::self()->uiControllerInternal()->arrangeBuddies() && !buddy && doc->mimeType().isValid()) {
                    // If buddy is not set, look for a (usually) plugin which handles this URL's mimetype
                    // and use its IBuddyDocumentFinder, if available, to find a buddy document
                    QString mime = doc->mimeType().name();
                    IBuddyDocumentFinder* buddyFinder = IBuddyDocumentFinder::finderForMimeType(mime);
                    if(buddyFinder) {
                        buddy = findBuddyDocument(url, buddyFinder);
                        if(buddy) {
                            placeAfterBuddy = buddyFinder->buddyOrder(buddy->url(), doc->url());
                        }
                    }
                }

                if(buddy) {
                    auto* sublimeDocBuddy = dynamic_cast<Sublime::Document*>(buddy);

                    if(sublimeDocBuddy) {
                        Sublime::AreaIndex *pActiveViewIndex = area->indexOf(uiController->activeSublimeWindow()->activeView());
                        if(pActiveViewIndex) {
                            // try to find existing View of buddy document in current active view's tab
                            const auto& activeAreaViews = pActiveViewIndex->views();
                            const auto& buddyViews = sublimeDocBuddy->views();
                            auto it = std::find_if(activeAreaViews.begin(), activeAreaViews.end(), [&](Sublime::View* view) {
                                return buddyViews.contains(view);
                            });
                            if (it != activeAreaViews.end()) {
                                buddyView = *it;
                            }
                        }
                    }
                }

                // add view to the area
                if(buddyView && area->indexOf(buddyView)) {
                    if(placeAfterBuddy) {
                        // Adding new view after buddy view, simple case
                        area->addView(partView, area->indexOf(buddyView), buddyView);
                    }
                    else {
                        // First new view, then buddy view
                        area->addView(partView, area->indexOf(buddyView), buddyView);
                        // move buddyView tab after the new document
                        area->removeView(buddyView);
                        area->addView(buddyView, area->indexOf(partView), partView);
                    }
                }
                else {
                    // no buddy found for new document / plugin does not support buddies / buddy feature disabled
                    Sublime::View *activeView = uiController->activeSublimeWindow()->activeView();
                    Sublime::UrlDocument *activeDoc = nullptr;
                    IBuddyDocumentFinder *buddyFinder = nullptr;
                    if(activeView)
                        activeDoc = qobject_cast<Sublime::UrlDocument *>(activeView->document());
                    if(activeDoc && Core::self()->uiControllerInternal()->arrangeBuddies()) {
                        const auto mime = activeDoc->mimeType().name();
                        buddyFinder = IBuddyDocumentFinder::finderForMimeType(mime);
                    }

                    if(Core::self()->uiControllerInternal()->openAfterCurrent() &&
                       Core::self()->uiControllerInternal()->arrangeBuddies() &&
                       buddyFinder)
                    {
                        // Check if active document's buddy is directly next to it.
                        // For example, we have the already-open tabs | *foo.h* | foo.cpp | , foo.h is active.
                        // When we open a new document here (and the buddy feature is enabled),
                        // we do not want to separate foo.h and foo.cpp, so we take care and avoid this.
                        Sublime::AreaIndex *activeAreaIndex = area->indexOf(activeView);
                        int pos = activeAreaIndex->views().indexOf(activeView);
                        Sublime::View *afterActiveView = activeAreaIndex->views().value(pos+1, nullptr);

                        Sublime::UrlDocument *activeDoc = nullptr, *afterActiveDoc = nullptr;
                        if(activeView && afterActiveView) {
                            activeDoc = qobject_cast<Sublime::UrlDocument *>(activeView->document());
                            afterActiveDoc = qobject_cast<Sublime::UrlDocument *>(afterActiveView->document());
                        }
                        if(activeDoc && afterActiveDoc &&
                           buddyFinder->areBuddies(activeDoc->url(), afterActiveDoc->url()))
                        {
                            // don't insert in between of two buddies, but after them
                            area->addView(partView, activeAreaIndex, afterActiveView);
                        }
                        else {
                            // The active document's buddy is not directly after it
                            // => no problem, insert after active document
                            area->addView(partView, activeView);
                        }
                    }
                    else {
                        // Opening as last tab won't disturb our buddies
                        // Same, if buddies are disabled, we needn't care about them.

                        // this method places the tab according to openAfterCurrent()
                        area->addView(partView, activeView);
                    }
                }
            }

            if (!activationParams.testFlag(IDocumentController::DoNotActivate))
            {
                uiController->activeSublimeWindow()->activateView(
                    partView, !activationParams.testFlag(IDocumentController::DoNotFocus));
            }
            if (!activationParams.testFlag(IDocumentController::DoNotAddToRecentOpen) && !controller->isEmptyDocumentUrl(url))
            {
                fileOpenRecent->addUrl( url );
            }

            if( range.isValid() )
            {
                if (range.isEmpty())
                    doc->setCursorPosition( range.start() );
                else
                    doc->setTextSelection( range );
            }
        }

        // Deferred signals, wait until it's all ready first
        if (wasClosed) {
            emit controller->documentOpened( doc );
        }

        if (!activationParams.testFlag(IDocumentController::DoNotActivate) && doc != controller->activeDocument())
            emit controller->documentActivated( doc );

        saveAll->setEnabled(true);
        revertAll->setEnabled(true);
        close->setEnabled(true);
        closeAll->setEnabled(true);
        closeAllOthers->setEnabled(true);

        KTextEditor::Cursor activePosition;
        if(range.isValid())
            activePosition = range.start();
        else if(KTextEditor::View* v = doc->activeTextView())
            activePosition = v->cursorPosition();

        if (doc != previousActiveDocument || activePosition != previousActivePosition)
            emit controller->documentJumpPerformed(doc, activePosition, previousActiveDocument, previousActivePosition);

        return true;
    }

    DocumentController* const controller;

    QPointer<QAction> saveAll;
    QPointer<QAction> revertAll;
    QPointer<QAction> close;
    QPointer<QAction> closeAll;
    QPointer<QAction> closeAllOthers;
    KRecentFilesAction* fileOpenRecent;
};
Q_DECLARE_TYPEINFO(KDevelop::DocumentControllerPrivate::HistoryEntry, Q_MOVABLE_TYPE);

DocumentController::DocumentController( QObject *parent )
        : IDocumentController( parent )
        , d_ptr(new DocumentControllerPrivate(this))
{
    setObjectName(QStringLiteral("DocumentController"));
    QDBusConnection::sessionBus().registerObject( QStringLiteral("/org/kdevelop/DocumentController"),
        this, QDBusConnection::ExportScriptableSlots );

    connect(this, &DocumentController::documentUrlChanged, this, [this](IDocument* document, const QUrl& previousUrl) {
        Q_D(DocumentController);
        d->changeDocumentUrl(document, previousUrl);
    });

    if(!(Core::self()->setupFlags() & Core::NoUi)) setupActions();
}

void DocumentController::initialize()
{
    Q_D(DocumentController);

    d->shuttingDown = false; // required by test_documentcontroller
}

void DocumentController::cleanup()
{
    Q_D(DocumentController);

    d->shuttingDown = true;

    if (d->fileOpenRecent)
        d->fileOpenRecent->saveEntries(KConfigGroup(KSharedConfig::openConfig(), QStringLiteral("Recent Files")));

    // Close all documents without checking if they should be saved.
    // This is because the user gets a chance to save them during MainWindow::queryClose.
    const auto documents = openDocuments();
    for (IDocument* doc : documents) {
        doc->close(IDocument::Discard);
    }
}

DocumentController::~DocumentController() = default;

void DocumentController::setupActions()
{
    Q_D(DocumentController);

    KActionCollection* ac = Core::self()->uiControllerInternal()->defaultMainWindow()->actionCollection();

    QAction* action;

    action = ac->addAction( QStringLiteral("file_open") );
    action->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
    ac->setDefaultShortcut(action, Qt::CTRL | Qt::Key_O);
    action->setText(i18nc("@action",  "&Open..." ) );
    connect(action, &QAction::triggered,
            this, [this] { Q_D(DocumentController); d->chooseDocument(); } );
    action->setToolTip( i18nc("@info:tooltip", "Open file" ) );
    action->setWhatsThis( i18nc("@info:whatsthis", "Opens a file for editing." ) );

    d->fileOpenRecent = KStandardAction::openRecent(this,
                    SLOT(slotOpenDocument(QUrl)), ac);
    d->fileOpenRecent->setWhatsThis(i18nc("@info:whatsthis", "This lists files which you have opened recently, and allows you to easily open them again."));
    d->fileOpenRecent->loadEntries(KConfigGroup(KSharedConfig::openConfig(), QStringLiteral("Recent Files")));

    action = d->saveAll = ac->addAction( QStringLiteral("file_save_all") );
    action->setIcon(QIcon::fromTheme(QStringLiteral("document-save")));
    action->setText(i18nc("@action", "Save Al&l" ) );
    connect( action, &QAction::triggered, this, &DocumentController::slotSaveAllDocuments );
    action->setToolTip( i18nc("@info:tooltip", "Save all open documents" ) );
    action->setWhatsThis( i18nc("@info:whatsthis", "Save all open documents, prompting for additional information when necessary." ) );
    ac->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::Key_L));
    action->setEnabled(false);

    action = d->revertAll = ac->addAction( QStringLiteral("file_revert_all") );
    action->setIcon(QIcon::fromTheme(QStringLiteral("document-revert")));
    action->setText(i18nc("@action", "Reload All" ) );
    connect( action, &QAction::triggered, this, &DocumentController::reloadAllDocuments );
    action->setToolTip( i18nc("@info:tooltip", "Revert all open documents" ) );
    action->setWhatsThis( i18nc("@info:whatsthis", "Revert all open documents, returning to the previously saved state." ) );
    action->setEnabled(false);

    action = d->close = ac->addAction( QStringLiteral("file_close") );
    action->setIcon(QIcon::fromTheme(QStringLiteral("document-close")));
    ac->setDefaultShortcut(action, Qt::CTRL | Qt::Key_W);
    action->setText( i18nc("@action", "&Close" ) );
    connect( action, &QAction::triggered, this, &DocumentController::fileClose );
    action->setToolTip( i18nc("@info:tooltip", "Close file" ) );
    action->setWhatsThis( i18nc("@info:whatsthis", "Closes current file." ) );
    action->setEnabled(false);

    action = d->closeAll = ac->addAction( QStringLiteral("file_close_all") );
    action->setIcon(QIcon::fromTheme(QStringLiteral("document-close")));
    action->setText(i18nc("@action", "Clos&e All" ) );
    connect( action, &QAction::triggered, this, &DocumentController::closeAllDocuments );
    action->setToolTip( i18nc("@info:tooltip", "Close all open documents" ) );
    action->setWhatsThis( i18nc("@info:whatsthis", "Close all open documents, prompting for additional information when necessary." ) );
    action->setEnabled(false);

    action = d->closeAllOthers = ac->addAction( QStringLiteral("file_closeother") );
    action->setIcon(QIcon::fromTheme(QStringLiteral("document-close")));
    ac->setDefaultShortcut(action, Qt::CTRL | Qt::SHIFT | Qt::Key_W);
    action->setText(i18nc("@action", "Close All Ot&hers" ) );
    connect( action, &QAction::triggered, this, &DocumentController::closeAllOtherDocuments );
    action->setToolTip( i18nc("@info:tooltip", "Close all other documents" ) );
    action->setWhatsThis( i18nc("@info:whatsthis", "Close all open documents, with the exception of the currently active document." ) );
    action->setEnabled(false);

    action = ac->addAction( QStringLiteral("vcsannotate_current_document") );
    connect( action, &QAction::triggered, this, &DocumentController::vcsAnnotateCurrentDocument );
    action->setText( i18nc("@action", "Show Annotate on Current Document") );
    action->setIconText( i18nc("@action", "Annotate" ) );
    action->setIcon( QIcon::fromTheme(QStringLiteral("user-properties")) );
}

void DocumentController::slotOpenDocument(const QUrl &url)
{
    openDocument(url);
}

IDocument* DocumentController::openDocumentFromText( const QString& data )
{
    IDocument* d = openDocument(nextEmptyDocumentUrl());
    Q_ASSERT(d->textDocument());
    d->textDocument()->setText( data );
    return d;
}

bool DocumentController::openDocumentFromTextSimple( QString text )
{
    return (bool)openDocumentFromText( text );
}

bool DocumentController::openDocumentSimple( QString url, int line, int column )
{
    return (bool)openDocument( QUrl::fromUserInput(url), KTextEditor::Cursor( line, column ) );
}

IDocument* DocumentController::openDocument( const QUrl& inputUrl, const QString& prefName )
{
    Q_D(DocumentController);

    return d->openDocumentInternal( inputUrl, prefName );
}

IDocument* DocumentController::openDocument( const QUrl & inputUrl,
        const KTextEditor::Range& range,
        DocumentActivationParams activationParams,
        const QString& encoding, IDocument* buddy)
{
    Q_D(DocumentController);

    if (d->shuttingDown) {
        // When a user exits KDevelop during debugging, a code breakpoint can be hit,
        // and as a consequence DebugController::showStepInSource() be called
        // in the event loop started by Core::cleanup() => BackgroundParser::waitForIdle().
        // Oblivious to the application state, DebugController then tries to open a document,
        // which eventually results in a crash inside a slot connected to either
        // &IDocumentController::textDocumentCreated or &IDocumentController::documentLoaded
        // (these signals are emitted in the process of opening a document).
        // Even had there been no crash, we should not open documents after cleanup(),
        // because we will never close them.
        qCDebug(SHELL) << "refusing to open document" << inputUrl << "after cleanup()";
        return nullptr;
    }

    return d->openDocumentInternal(inputUrl, QString(), range, encoding, activationParams, buddy);
}


bool DocumentController::openDocument(IDocument* doc,
                                      const KTextEditor::Range& range,
                                      DocumentActivationParams activationParams,
                                      IDocument* buddy)
{
    Q_D(DocumentController);

    return d->openDocumentInternal( doc, range, activationParams, buddy);
}


void DocumentController::fileClose()
{
    IDocument *activeDoc = activeDocument();
    if (activeDoc)
    {
        UiController *uiController = Core::self()->uiControllerInternal();
        Sublime::View *activeView = uiController->activeSublimeWindow()->activeView();

        uiController->activeArea()->closeView(activeView);
    }
}

void DocumentController::notifyDocumentClosed(Sublime::Document* doc_)
{
    Q_D(DocumentController);

    auto* doc = qobject_cast<IDocument*>(doc_);
    Q_ASSERT(doc);

    d->removeDocument(doc_);

    if (d->documents.isEmpty()) {
        if (d->saveAll)
            d->saveAll->setEnabled(false);
        if (d->revertAll)
            d->revertAll->setEnabled(false);
        if (d->close)
            d->close->setEnabled(false);
        if (d->closeAll)
            d->closeAll->setEnabled(false);
        if (d->closeAllOthers)
            d->closeAllOthers->setEnabled(false);
    }

    emit documentClosed(doc);
}

bool DocumentController::isUntitledDocumentUrl(const QUrl& url) const
{
    return isEmptyDocumentUrl(url);
}

IDocument * DocumentController::documentForUrl( const QUrl & dirtyUrl ) const
{
    Q_D(const DocumentController);

    if (dirtyUrl.isEmpty()) {
        return nullptr;
    }
    Q_ASSERT(!dirtyUrl.isRelative());
    Q_ASSERT(!dirtyUrl.fileName().isEmpty() || !dirtyUrl.isLocalFile());
    //Fix urls that might not be normalized
    return d->documents.value( dirtyUrl.adjusted( QUrl::NormalizePathSegments ), nullptr );
}

QList<IDocument*> DocumentController::openDocuments() const
{
    Q_D(const DocumentController);

    QList<IDocument*> opened;
    for (IDocument* doc : std::as_const(d->documents)) {
        auto *sdoc = dynamic_cast<Sublime::Document*>(doc);
        if( !sdoc )
        {
            continue;
        }
        if (!sdoc->views().isEmpty())
            opened << doc;
    }
    return opened;
}

void DocumentController::activateDocument( IDocument * document, const KTextEditor::Range& range )
{
    // TODO avoid some code in openDocument?
    Q_ASSERT(document);
    openDocument(document->url(), range, IDocumentController::DoNotAddToRecentOpen);
}

void DocumentController::slotSaveAllDocuments()
{
    saveAllDocuments(IDocument::Silent);
}

bool DocumentController::saveAllDocuments(IDocument::DocumentSaveMode mode)
{
    return saveSomeDocuments(openDocuments(), mode);
}

bool KDevelop::DocumentController::saveSomeDocuments(const QList< IDocument * > & list, IDocument::DocumentSaveMode mode)
{
    if (mode & IDocument::Silent) {
        const auto documents = modifiedDocuments(list);
        for (IDocument* doc : documents) {
            if( !DocumentController::isEmptyDocumentUrl(doc->url()) && !doc->save(mode) )
            {
                if( doc )
                    qCWarning(SHELL) << "!! Could not save document:" << doc->url();
                else
                    qCWarning(SHELL) << "!! Could not save document as its NULL";
            }
            // TODO if (!ret) showErrorDialog() ?
        }

    } else {
        // Ask the user which documents to save
        QList<IDocument*> checkSave = modifiedDocuments(list);

        if (!checkSave.isEmpty()) {
            ScopedDialog<KSaveSelectDialog> dialog(checkSave, qApp->activeWindow());
            return dialog->exec();
        }
    }

    return true;
}

QList< IDocument * > KDevelop::DocumentController::visibleDocumentsInWindow(MainWindow * mw) const
{
    // Gather a list of all documents which do have a view in the given main window
    // Does not find documents which are open in inactive areas
    QList<IDocument*> list;
    const auto documents = openDocuments();
    for (IDocument* doc : documents) {
        if (auto* sdoc = dynamic_cast<Sublime::Document*>(doc)) {
            const auto views = sdoc->views();
            auto hasViewInWindow = std::any_of(views.begin(), views.end(), [&](Sublime::View* view) {
                return (view->hasWidget() && view->widget()->window() == mw);
            });
            if (hasViewInWindow) {
                list.append(doc);
            }
        }
    }
    return list;
}

QList< IDocument * > KDevelop::DocumentController::documentsExclusivelyInWindow(MainWindow * mw, bool currentAreaOnly) const
{
    // Gather a list of all documents which have views only in the given main window
    QList<IDocument*> checkSave;

    const auto documents = openDocuments();
    for (IDocument* doc : documents) {
        if (auto* sdoc = dynamic_cast<Sublime::Document*>(doc)) {
            bool inOtherWindow = false;

            const auto views = sdoc->views();
            for (Sublime::View* view : views) {
                const auto windows = Core::self()->uiControllerInternal()->mainWindows();
                for (Sublime::MainWindow* window : windows) {
                    if(window->containsView(view) && (window != mw || (currentAreaOnly && window == mw && !mw->area()->views().contains(view)))) {
                        inOtherWindow = true;
                        break;
                    }
                }
                if (inOtherWindow) {
                    break;
                }
            }

            if (!inOtherWindow)
                checkSave.append(doc);
        }
    }
    return checkSave;
}

QList< IDocument * > KDevelop::DocumentController::modifiedDocuments(const QList< IDocument * > & list) const
{
    QList< IDocument * > ret;
    for (IDocument* doc : list) {
        if (doc->state() == IDocument::Modified || doc->state() == IDocument::DirtyAndModified)
            ret.append(doc);
    }
    return ret;
}

bool DocumentController::saveAllDocumentsForWindow(KParts::MainWindow* mw, KDevelop::IDocument::DocumentSaveMode mode, bool currentAreaOnly)
{
    QList<IDocument*> checkSave = documentsExclusivelyInWindow(qobject_cast<KDevelop::MainWindow*>(mw), currentAreaOnly);

    return saveSomeDocuments(checkSave, mode);
}

void DocumentController::reloadAllDocuments()
{
    if (Sublime::MainWindow* mw = Core::self()->uiControllerInternal()->activeSublimeWindow()) {
        const QList<IDocument*> views = visibleDocumentsInWindow(qobject_cast<KDevelop::MainWindow*>(mw));

        if (!saveSomeDocuments(views, IDocument::Default))
            // User cancelled or other error
            return;

        for (IDocument* doc : views) {
            if(!isEmptyDocumentUrl(doc->url()))
                doc->reload();
        }
    }
}

bool DocumentController::closeAllDocuments()
{
    if (Sublime::MainWindow* mw = Core::self()->uiControllerInternal()->activeSublimeWindow()) {
        const QList<IDocument*> views = visibleDocumentsInWindow(qobject_cast<KDevelop::MainWindow*>(mw));

        if (!saveSomeDocuments(views, IDocument::Default))
            // User cancelled or other error
            return false;

        for (IDocument* doc : views) {
            doc->close(IDocument::Discard);
        }
    }
    return true;
}

void DocumentController::closeAllOtherDocuments()
{
    if (Sublime::MainWindow* mw = Core::self()->uiControllerInternal()->activeSublimeWindow()) {
        Sublime::View* activeView = mw->activeView();

        if (!activeView) {
            qCWarning(SHELL) << "Shouldn't there always be an active view when this function is called?";
            return;
        }

        // Deal with saving unsaved solo views
        QList<IDocument*> soloViews = documentsExclusivelyInWindow(qobject_cast<KDevelop::MainWindow*>(mw));
        soloViews.removeAll(qobject_cast<IDocument*>(activeView->document()));

        if (!saveSomeDocuments(soloViews, IDocument::Default))
            // User cancelled or other error
            return;

        const auto views = mw->area()->views();
        for (Sublime::View* view : views) {
            if (view != activeView)
                mw->area()->closeView(view);
        }
        activeView->widget()->setFocus();
    }
}

IDocument* DocumentController::activeDocument() const
{
    UiController *uiController = Core::self()->uiControllerInternal();
    Sublime::MainWindow* mw = uiController->activeSublimeWindow();
    if( !mw || !mw->activeView() ) return nullptr;
    return qobject_cast<IDocument*>(mw->activeView()->document());
}

KTextEditor::View* DocumentController::activeTextDocumentView() const
{
    UiController *uiController = Core::self()->uiControllerInternal();
    Sublime::MainWindow* mw = uiController->activeSublimeWindow();
    if( !mw || !mw->activeView() )
        return nullptr;

    auto* view = qobject_cast<TextView*>(mw->activeView());
    if(!view)
        return nullptr;
    return view->textView();
}

QString DocumentController::activeDocumentPath( const QString& target ) const
{
    if(!target.isEmpty()) {
        const auto projects = Core::self()->projectController()->projects();
        for (IProject* project : projects) {
            if(project->name().startsWith(target, Qt::CaseInsensitive)) {
                return project->path().pathOrUrl() + QLatin1String("/.");
            }
        }
    }
    IDocument* doc = activeDocument();
    if(!doc || target == QLatin1String("[selection]"))
    {
        Context* selection = ICore::self()->selectionController()->currentSelection();
        if(selection && selection->type() == Context::ProjectItemContext && !static_cast<ProjectItemContext*>(selection)->items().isEmpty())
        {
            QString ret = static_cast<ProjectItemContext*>(selection)->items().at(0)->path().pathOrUrl();
            if(static_cast<ProjectItemContext*>(selection)->items().at(0)->folder())
                ret += QLatin1String("/.");
            return  ret;
        }
        return QString();
    }
    return doc->url().toString();
}

QStringList DocumentController::activeDocumentPaths() const
{
    UiController *uiController = Core::self()->uiControllerInternal();
    if( !uiController->activeSublimeWindow() ) return QStringList();

    QSet<QString> documents;
    const auto views = uiController->activeSublimeWindow()->area()->views();
    for (Sublime::View* view : views) {
        documents.insert(view->document()->documentSpecifier());
    }

    return documents.values();
}

void DocumentController::registerDocumentForMimetype( const QString& mimetype,
                                        KDevelop::IDocumentFactory* factory )
{
    Q_D(DocumentController);

    if( !d->factories.contains( mimetype ) )
        d->factories[mimetype] = factory;
}

QStringList DocumentController::documentTypes() const
{
    return QStringList() << QStringLiteral("Text");
}

static const QRegularExpression& emptyDocumentPattern()
{
    static const QRegularExpression pattern(QStringLiteral("^/%1(?:\\s\\((\\d+)\\))?$").arg(EMPTY_DOCUMENT_URL));
    return pattern;
}

bool DocumentController::isEmptyDocumentUrl(const QUrl &url)
{
    return emptyDocumentPattern().match(url.toDisplayString(QUrl::PreferLocalFile)).hasMatch();
}

QUrl DocumentController::nextEmptyDocumentUrl()
{
    int nextEmptyDocNumber = 0;
    const auto& pattern = emptyDocumentPattern();
    const auto openDocuments = Core::self()->documentControllerInternal()->openDocuments();
    for (IDocument* doc : openDocuments) {
        if (DocumentController::isEmptyDocumentUrl(doc->url())) {
            const auto match = pattern.match(doc->url().toDisplayString(QUrl::PreferLocalFile));
            if (match.hasMatch()) {
                const auto num = match.capturedView(1).toInt();
                nextEmptyDocNumber = qMax(nextEmptyDocNumber, num + 1);
            } else {
                nextEmptyDocNumber = qMax(nextEmptyDocNumber, 1);
            }
        }
    }

    QUrl url;
    if (nextEmptyDocNumber > 0)
        url = QUrl::fromLocalFile(QStringLiteral("/%1 (%2)").arg(EMPTY_DOCUMENT_URL).arg(nextEmptyDocNumber));
    else
        url = QUrl::fromLocalFile(QLatin1Char('/') + EMPTY_DOCUMENT_URL);
    return url;
}

IDocumentFactory* DocumentController::factory(const QString& mime) const
{
    Q_D(const DocumentController);

    return d->factories.value(mime);
}

bool DocumentController::openDocumentsSimple( QStringList urls )
{
    Sublime::Area* area = Core::self()->uiControllerInternal()->activeArea();
    Sublime::AreaIndex* areaIndex = area->rootIndex();

    QList<Sublime::View*> topViews = static_cast<Sublime::MainWindow*>(Core::self()->uiControllerInternal()->activeMainWindow())->topViews();

    if(Sublime::View* activeView = Core::self()->uiControllerInternal()->activeSublimeWindow()->activeView())
        areaIndex = area->indexOf(activeView);

    qCDebug(SHELL) << "opening " << urls << " to area " << area << " index " << areaIndex << " with children " << areaIndex->first() << " " << areaIndex->second();

    bool isFirstView = true;

    bool ret = openDocumentsWithSplitSeparators( areaIndex, urls, isFirstView );

    qCDebug(SHELL) << "area arch. after opening: " << areaIndex->print();

    // Required because sublime sometimes doesn't update correctly when the area-index contents has been changed
    // (especially when views have been moved to other indices, through unsplit, split, etc.)
    static_cast<Sublime::MainWindow*>(Core::self()->uiControllerInternal()->activeMainWindow())->reconstructViews(topViews);

    return ret;
}

bool DocumentController::openDocumentsWithSplitSeparators( Sublime::AreaIndex* index, QStringList urlsWithSeparators, bool& isFirstView )
{
    qCDebug(SHELL) << "opening " << urlsWithSeparators << " index " << index << " with children " << index->first() << " " << index->second() << " view-count " << index->viewCount();
    if(urlsWithSeparators.isEmpty())
        return true;

    Sublime::Area* area = Core::self()->uiControllerInternal()->activeArea();

    QList<int> topLevelSeparators; // Indices of the top-level separators (with groups skipped)
    const QStringList separators {QStringLiteral("/"), QStringLiteral("-")};
    QList<QStringList> groups;

    bool ret = true;

    {
        int parenDepth = 0;
        int groupStart = 0;
        for(int pos = 0; pos < urlsWithSeparators.size(); ++pos)
        {
            QString item = urlsWithSeparators[pos];
            if(separators.contains(item))
            {
                if(parenDepth == 0)
                    topLevelSeparators << pos;
            }else if(item == QLatin1String("["))
            {
                if(parenDepth == 0)
                    groupStart = pos+1;
                ++parenDepth;
            }
            else if(item == QLatin1String("]"))
            {
                if(parenDepth > 0)
                {
                    --parenDepth;

                    if(parenDepth == 0)
                        groups << urlsWithSeparators.mid(groupStart, pos-groupStart);
                }
                else{
                    qCDebug(SHELL) << "syntax error in " << urlsWithSeparators << ": parens do not match";
                    ret = false;
                }
            }else if(parenDepth == 0)
            {
                groups << (QStringList() << item);
            }
        }
    }

    if(topLevelSeparators.isEmpty())
    {
        if(urlsWithSeparators.size() > 1)
        {
            for (const QStringList& group : std::as_const(groups)) {
                ret &= openDocumentsWithSplitSeparators( index, group, isFirstView );
            }
        }else{
            const auto url = QUrl::fromUserInput(urlsWithSeparators.front());

            // The file name of a remote URL that ends with a slash is empty, but such a URL can still reference a file.
            // The opposite condition is asserted in DocumentControllerPrivate::openDocumentInternal(),
            // which is (indirectly) called below.
            if (url.isLocalFile() && url.fileName().isEmpty()) {
                qCDebug(SHELL) << "cannot open a directory" << url.toString();
                return false;
            }

            while(index->isSplit())
                index = index->first();
            // Simply open the document into the area index
            IDocument* doc = Core::self()->documentControllerInternal()->openDocument(
                url, KTextEditor::Cursor::invalid(),
                IDocumentController::DoNotActivate | IDocumentController::DoNotCreateView);
            auto *sublimeDoc = dynamic_cast<Sublime::Document*>(doc);
            if (sublimeDoc) {
                Sublime::View* view = sublimeDoc->createView();
                area->addView(view, index);
                if(isFirstView)
                {
                    static_cast<Sublime::MainWindow*>(Core::self()->uiControllerInternal()->activeMainWindow())->activateView(view);
                    isFirstView = false;
                }
            }else{
                ret = false;
            }
        }
        return ret;
    }

    // Pick a separator in the middle

    int pickSeparator = topLevelSeparators[topLevelSeparators.size()/2];

    bool activeViewToSecondChild = false;
    if(pickSeparator == urlsWithSeparators.size()-1)
    {
        // There is no right child group, so the right side should be filled with the currently active views
        activeViewToSecondChild = true;
    }else{
        QStringList separatorsAndParens = separators;
        separatorsAndParens << QStringLiteral("[") << QStringLiteral("]");
        // Check if the second child-set contains an unterminated separator, which means that the active views should end up there
        for(int pos = pickSeparator+1; pos < urlsWithSeparators.size(); ++pos)
            if( separators.contains(urlsWithSeparators[pos]) && (pos == urlsWithSeparators.size()-1 ||
                separatorsAndParens.contains(urlsWithSeparators[pos-1])) )
                    activeViewToSecondChild = true;
    }

    Qt::Orientation orientation = urlsWithSeparators[pickSeparator] == QLatin1String("/") ? Qt::Horizontal : Qt::Vertical;

    if(!index->isSplit())
    {
        qCDebug(SHELL) << "splitting " << index << "orientation" << orientation << "to second" << activeViewToSecondChild;
        index->split(orientation, activeViewToSecondChild);
    }else{
        index->setOrientation(orientation);
        qCDebug(SHELL) << "WARNING: Area is already split (shouldn't be)" << urlsWithSeparators;
    }

    openDocumentsWithSplitSeparators( index->first(), urlsWithSeparators.mid(0, pickSeparator) , isFirstView );
    if(pickSeparator != urlsWithSeparators.size() - 1)
        openDocumentsWithSplitSeparators( index->second(), urlsWithSeparators.mid(pickSeparator+1, urlsWithSeparators.size() - (pickSeparator+1) ), isFirstView );

    // Clean up the child-indices, because document-loading may fail

    if(!index->first()->viewCount() && !index->first()->isSplit())
    {
        qCDebug(SHELL) << "unsplitting first";
        index->unsplit(index->first());
    }
    else if(!index->second()->viewCount() && !index->second()->isSplit())
    {
        qCDebug(SHELL) << "unsplitting second";
        index->unsplit(index->second());
    }

    return ret;
}

void DocumentController::vcsAnnotateCurrentDocument()
{
    IDocument* doc = activeDocument();
    if (!doc)
        return;

    QUrl url = doc->url();
    IProject* project = KDevelop::ICore::self()->projectController()->findProjectForUrl(url);
    if(project && project->versionControlPlugin()) {
        auto* iface = project->versionControlPlugin()->extension<IBasicVersionControl>();
        auto helper = new VcsPluginHelper(project->versionControlPlugin(), iface);
        connect(doc->textDocument(), &KTextEditor::Document::aboutToClose, helper,
                qOverload<KTextEditor::Document*>(&VcsPluginHelper::disposeEventually));
        connect(doc->activeTextView(), &KTextEditor::View::annotationBorderVisibilityChanged, helper,
                qOverload<KTextEditor::View*, bool>(&VcsPluginHelper::disposeEventually));
        helper->addContextDocument(url);
        helper->annotation();
    }
    else {
        const QString messageText =
            i18n("Could not annotate the document because it is not part of a version-controlled project.");
        auto* message = new Sublime::Message(messageText, Sublime::Message::Error);
        ICore::self()->uiController()->postMessage(message);
    }
}

#include "moc_documentcontroller.cpp"
