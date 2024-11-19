/*
    SPDX-FileCopyrightText: 2006-2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "patchreview.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTimer>
#include <QMimeDatabase>

#include <KActionCollection>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KMessageBox>
#include <KIO/CopyJob>

#include <interfaces/idocument.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/context.h>
#include <interfaces/editorcontext.h>

#include <project/projectmodel.h>

#include <sublime/message.h>
#include <util/path.h>

#include <KompareDiff2/DiffSettings>
#include <KompareDiff2/Global>
#include <KompareDiff2/ModelList>

#include <KTextEditor/Document>
#include <KTextEditor/MovingRange>
#include <KTextEditor/View>

///Whether arbitrary exceptions that occurred while diff-parsing within the library should be caught
#define CATCHLIBDIFF

/* Exclude this file from doublequote_chars check as krazy doesn't understand
   std::string*/
//krazy:excludeall=doublequote_chars
#include <sublime/controller.h>
#include <sublime/mainwindow.h>
#include <sublime/area.h>
#include <sublime/document.h>
#include <sublime/view.h>
#include <vcs/widgets/vcsdiffpatchsources.h>
#include "patchhighlighter.h"
#include "patchreviewtoolview.h"
#include "localpatchsource.h"
#include "debug.h"


using namespace KDevelop;

namespace
{
// Maximum number of files to open directly within a tab when the review is started
const int maximumFilesToOpenDirectly = 15;
}

void PatchReviewPlugin::seekHunk( bool forwards, const QUrl& fileName ) {
    try {
        qCDebug(PLUGIN_PATCHREVIEW) << forwards << fileName << fileName.isEmpty();
        if ( !m_modelList )
            throw "no model";

        for ( int a = 0; a < m_modelList->modelCount(); ++a ) {
            const auto* const model = m_modelList->modelAt(a);
            if ( !model || !model->differences() )
                continue;

            QUrl file = urlForFileModel( model );

            if ( !fileName.isEmpty() && fileName != file )
                continue;

            IDocument* doc = ICore::self()->documentController()->documentForUrl( file );

            if ( doc && m_highlighters.contains( doc->url() ) && m_highlighters[doc->url()] ) {
                if ( doc->textDocument() ) {
                    const QList<KTextEditor::MovingRange*> ranges = m_highlighters[doc->url()]->ranges();

                    KTextEditor::View * v = doc->activeTextView();
                    if ( v ) {
                        int bestLine = -1;
                        KTextEditor::Cursor c = v->cursorPosition();
                        for (auto* range : ranges) {
                            const int line = range->start().line();

                            if ( forwards ) {
                                if ( line > c.line() && ( bestLine == -1 || line < bestLine ) )
                                    bestLine = line;
                            } else {
                                if ( line < c.line() && ( bestLine == -1 || line > bestLine ) )
                                    bestLine = line;
                            }
                        }
                        if ( bestLine != -1 ) {
                            v->setCursorPosition( KTextEditor::Cursor( bestLine, 0 ) );
                            return;
                        } else if(fileName.isEmpty()) {
                            int next = qBound(0, forwards ? a+1 : a-1, m_modelList->modelCount()-1);
                            if (next < maximumFilesToOpenDirectly) {
                                ICore::self()->documentController()->openDocument(urlForFileModel(m_modelList->modelAt(next)));
                            }
                        }
                    }
                }
            }
        }
    } catch ( const QString & str ) {
        qCDebug(PLUGIN_PATCHREVIEW) << "seekHunk():" << str;
    } catch ( const char * str ) {
        qCDebug(PLUGIN_PATCHREVIEW) << "seekHunk():" << str;
    }
    qCDebug(PLUGIN_PATCHREVIEW) << "no matching hunk found";
}

void PatchReviewPlugin::addHighlighting( const QUrl& highlightFile, IDocument* document ) {
    try {
        if ( !modelList() )
            throw "no model";

        for ( int a = 0; a < modelList()->modelCount(); ++a ) {
            auto* const model = modelList()->modelAt(a);
            if ( !model )
                continue;

            QUrl file = urlForFileModel( model );

            if ( file != highlightFile )
                continue;

            qCDebug(PLUGIN_PATCHREVIEW) << "highlighting" << file.toDisplayString();

            IDocument* doc = document;
            if( !doc )
                doc = ICore::self()->documentController()->documentForUrl( file );

            qCDebug(PLUGIN_PATCHREVIEW) << "highlighting file" << file << "with doc" << doc;

            if ( !doc || !doc->textDocument() )
                continue;

            removeHighlighting( file );

            m_highlighters[file] = new PatchHighlighter(model, doc, this, (qobject_cast<LocalPatchSource*>(m_patch.data()) == nullptr));
        }
    } catch ( const QString & str ) {
        qCDebug(PLUGIN_PATCHREVIEW) << "highlightFile():" << str;
    } catch ( const char * str ) {
        qCDebug(PLUGIN_PATCHREVIEW) << "highlightFile():" << str;
    }
}

void PatchReviewPlugin::highlightPatch() {
    try {
        if ( !modelList() )
            throw "no model";

        for ( int a = 0; a < modelList()->modelCount(); ++a ) {
            const auto* const model = modelList()->modelAt(a);
            if ( !model )
                continue;

            QUrl file = urlForFileModel( model );

            addHighlighting( file );
        }
    } catch ( const QString & str ) {
        qCDebug(PLUGIN_PATCHREVIEW) << "highlightFile():" << str;
    } catch ( const char * str ) {
        qCDebug(PLUGIN_PATCHREVIEW) << "highlightFile():" << str;
    }
}

void PatchReviewPlugin::removeHighlighting( const QUrl& file ) {
    if ( file.isEmpty() ) {
        ///Remove all highlighting
        qDeleteAll( m_highlighters );
        m_highlighters.clear();
    } else {
        HighlightMap::iterator it = m_highlighters.find( file );
        if ( it != m_highlighters.end() ) {
            delete *it;
            m_highlighters.erase( it );
        }
    }
}

void PatchReviewPlugin::notifyPatchChanged() {
    if (m_patch) {
        qCDebug(PLUGIN_PATCHREVIEW) << "notifying patch change: " << m_patch->file();
        m_updateKompareTimer->start();
    } else {
        m_updateKompareTimer->stop();
    }
}

void PatchReviewPlugin::forceUpdate() {
    if( m_patch ) {
        // don't trigger an update if we know the plugin cannot update itself
        auto* vcsPatch = qobject_cast<VCSDiffPatchSource*>(m_patch.data());
        if (!vcsPatch || vcsPatch->m_updater) {
            m_patch->update();
            notifyPatchChanged();
        }
    }
}

void PatchReviewPlugin::updateKompareModel() {
    if ( !m_patch ) {
        ///TODO: this method should be cleaned up, it can be called by the timer and
        ///      e.g. https://bugs.kde.org/show_bug.cgi?id=267187 shows how it could
        ///      lead to asserts before...
        return;
    }

    qCDebug(PLUGIN_PATCHREVIEW) << "updating model";
    removeHighlighting();
    m_modelList.reset( nullptr );
    m_depth = 0;
    m_diffSettings.reset(nullptr);
    {
        IDocument* patchDoc = ICore::self()->documentController()->documentForUrl( m_patch->file() );
        if( patchDoc )
            patchDoc->reload();
    }

    QString patchFile;
    if( m_patch->file().isLocalFile() )
        patchFile = m_patch->file().toLocalFile();
    else if( m_patch->file().isValid() && !m_patch->file().isEmpty() ) {
        patchFile = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        bool ret = KIO::copy(m_patch->file(), QUrl::fromLocalFile(patchFile), KIO::HideProgressInfo)->exec();
        if( !ret ) {
            qCWarning(PLUGIN_PATCHREVIEW) << "Problem while downloading: " << m_patch->file() << "to" << patchFile;
            patchFile.clear();
        }
    }

    if (!patchFile.isEmpty()) //only try to construct the model if we have a patch to load
    try {
        m_diffSettings.reset(new KompareDiff2::DiffSettings());
        m_kompareInfo.reset(new KompareDiff2::Info());
        m_kompareInfo->localDestination = patchFile;
        m_kompareInfo->localSource = m_patch->baseDir().toLocalFile();
        m_kompareInfo->depth = m_patch->depth();
        m_kompareInfo->applied = m_patch->isAlreadyApplied();

        m_modelList.reset(new KompareDiff2::ModelList(m_diffSettings.data(), this));
        m_modelList->slotKompareInfo( m_kompareInfo.data() );

        try {
            m_modelList->openDirAndDiff();
        } catch ( const QString & /*str*/ ) {
            throw;
        } catch ( ... ) {
            throw QStringLiteral( "lib/libdiff2 crashed, memory may be corrupted. Please restart kdevelop." );
        }

        for (m_depth = 0; m_depth < 10; ++m_depth) {
            bool allFound = true;
            for( int i = 0; i < m_modelList->modelCount(); i++ ) {
                if (!QFile::exists(urlForFileModel(m_modelList->modelAt(i)).toLocalFile())) {
                    allFound = false;
                }
            }
            if (allFound) {
                break; // found depth
            }
        }

        emit patchChanged();

        for( int i = 0; i < m_modelList->modelCount(); i++ ) {
            const auto* const model = m_modelList->modelAt(i);
            for (auto* difference : *model->differences()) {
                difference->apply(m_patch->isAlreadyApplied());
            }
        }

        highlightPatch();

        return;
    } catch ( const QString & str ) {
        KMessageBox::error(nullptr, str, i18nc("@title:window", "Kompare Model Update"));
    } catch ( const char * str ) {
        KMessageBox::error(nullptr, QLatin1String(str), i18nc("@title:window", "Kompare Model Update"));
    }
    removeHighlighting();
    m_modelList.reset( nullptr );
    m_depth = 0;
    m_kompareInfo.reset( nullptr );
    m_diffSettings.reset(nullptr);

    emit patchChanged();
}

K_PLUGIN_FACTORY_WITH_JSON(KDevPatchReviewFactory, "kdevpatchreview.json",
                           registerPlugin<PatchReviewPlugin>();)

class PatchReviewToolViewFactory : public KDevelop::IToolViewFactory
{
public:
    explicit PatchReviewToolViewFactory( PatchReviewPlugin *plugin ) : m_plugin( plugin ) {}

    QWidget* create( QWidget *parent = nullptr ) override {
        return new PatchReviewToolView( parent, m_plugin );
    }

    Qt::DockWidgetArea defaultPosition() const override
    {
        return Qt::BottomDockWidgetArea;
    }

    QString id() const override {
        return QStringLiteral("org.kdevelop.PatchReview");
    }

private:
    PatchReviewPlugin *m_plugin;
};

PatchReviewPlugin::~PatchReviewPlugin()
{
    removeHighlighting();

    // Tweak to work around a crash on OS X; see https://bugs.kde.org/show_bug.cgi?id=338829
    // and http://qt-project.org/forums/viewthread/38406/#162801
    // modified tweak: use setPatch() and deleteLater in that method.
    setPatch(nullptr);
}

void PatchReviewPlugin::closeReview()
{
    if( m_patch ) {
        IDocument* patchDocument = ICore::self()->documentController()->documentForUrl( m_patch->file() );
        if (patchDocument) {
            // Revert modifications to the text document which we've done in updateReview
            patchDocument->setPrettyName( QString() );
            patchDocument->textDocument()->setReadWrite( true );
            patchDocument->textDocument()->setModifiedOnDiskWarning(true);
        }

        removeHighlighting();
        m_modelList.reset( nullptr );
        m_depth = 0;

        if (!qobject_cast<LocalPatchSource*>(m_patch.data())) {
            // make sure "show" button still openes the file dialog to open a custom patch file
            setPatch( new LocalPatchSource );
        } else
            emit patchChanged();

        auto oldArea = ICore::self()->uiController()->activeArea();
        if (oldArea->objectName() == QLatin1String("review")) {
            if (ICore::self()->documentController()->saveAllDocumentsForWindow(
                    ICore::self()->uiController()->activeMainWindow(),
                    IDocumentController::SaveSelectionMode::LetUserSelect, true)) {
                ICore::self()->uiController()->switchToArea(m_lastArea.isEmpty() ? QStringLiteral("code") : m_lastArea,
                                                            KDevelop::IUiController::ThisWindow);
                if (oldArea->workingSetPersistent()) {
                    ICore::self()->uiController()->activeArea()->setWorkingSet(oldArea->workingSet(), true, oldArea);
                }
            }
        }
    }
}

void PatchReviewPlugin::cancelReview() {
    if( m_patch ) {
        m_patch->cancelReview();
        closeReview();
    }
}

void PatchReviewPlugin::finishReview(const QList<QUrl>& selection)
{
    if( m_patch && m_patch->finishReview( selection ) ) {
        closeReview();
    }
}

void PatchReviewPlugin::startReview( IPatchSource* patch, IPatchReview::ReviewMode mode ) {
    Q_UNUSED( mode );
    emit startingNewReview();
    setPatch( patch );
    QMetaObject::invokeMethod(this, &PatchReviewPlugin::updateReview, Qt::QueuedConnection);
}

void PatchReviewPlugin::switchToEmptyReviewArea()
{
    const auto allAreas = ICore::self()->uiController()->allAreas();
    for (Sublime::Area* area : allAreas) {
        if (area->objectName() == QLatin1String("review")) {
            area->setWorkingSet(QString(), false);
        }
    }

    QString areaName = ICore::self()->uiController()->activeArea()->objectName();
    if (areaName != QLatin1String("review")) {
        m_lastArea = areaName;
        ICore::self()->uiController()->switchToArea(QStringLiteral("review"), KDevelop::IUiController::ThisWindow);
    } else {
        m_lastArea.clear();
    }
}

QUrl PatchReviewPlugin::urlForFileModel(const KompareDiff2::DiffModel* model) const
{
    KDevelop::Path path(QDir::cleanPath(m_patch->baseDir().toLocalFile()));
    QVector<QString> destPath = KDevelop::Path(QLatin1Char('/') + model->destinationPath()).segments();
    if (destPath.size() >= (int)m_depth) {
        destPath.remove(0, m_depth);
    }
    for (const QString& segment : std::as_const(destPath)) {
        path.addPath(segment);
    }
    path.addPath(model->destinationFile());

    return path.toUrl();
}

void PatchReviewPlugin::updateReview()
{
    if( !m_patch )
        return;

    m_updateKompareTimer->stop();

    switchToEmptyReviewArea();

    KDevelop::IDocumentController *docController = ICore::self()->documentController();
    // don't add documents opened automatically to the Files/Open Recent list
    IDocument* futureActiveDoc = docController->openDocument( m_patch->file(), KTextEditor::Range::invalid(),
                                                              IDocumentController::DoNotAddToRecentOpen );

    updateKompareModel();

    if ( !m_modelList || !futureActiveDoc || !futureActiveDoc->textDocument() ) {
        // might happen if e.g. openDocument dialog was cancelled by user
        // or under the theoretic possibility of a non-text document getting opened
        return;
    }

    futureActiveDoc->textDocument()->setReadWrite( false );
    futureActiveDoc->setPrettyName(i18nc("@title complete patch", "Overview"));
    futureActiveDoc->textDocument()->setModifiedOnDiskWarning(false);

    docController->activateDocument( futureActiveDoc );

    auto* toolView = qobject_cast<PatchReviewToolView*>(ICore::self()->uiController()->findToolView(i18nc("@title:window", "Patch Review"), m_factory));
    Q_ASSERT( toolView );

    //Open all relates files
    for( int a = 0; a < m_modelList->modelCount() && a < maximumFilesToOpenDirectly; ++a ) {
        QUrl absoluteUrl = urlForFileModel( m_modelList->modelAt( a ) );
        if (absoluteUrl.isRelative()) {
            const QString messageText = i18n("The base directory of the patch must be an absolute directory.");
            auto* message = new Sublime::Message(messageText, Sublime::Message::Error);
            ICore::self()->uiController()->postMessage(message);
            break;
        }

        if( QFileInfo::exists( absoluteUrl.toLocalFile() ) && absoluteUrl.toLocalFile() != QLatin1String("/dev/null") )
        {
            toolView->open( absoluteUrl, false );
        }else{
            // Maybe the file was deleted
            qCDebug(PLUGIN_PATCHREVIEW) << "could not open" << absoluteUrl << "because it doesn't exist";
        }
    }
}

void PatchReviewPlugin::setPatch( IPatchSource* patch ) {
    if ( patch == m_patch ) {
        return;
    }

    if( m_patch ) {
        disconnect( m_patch.data(), &IPatchSource::patchChanged, this, &PatchReviewPlugin::notifyPatchChanged );
        m_patch->deleteLater();
    }
    m_patch = patch;

    if( m_patch ) {
        qCDebug(PLUGIN_PATCHREVIEW) << "setting new patch" << patch->name() << "with file" << patch->file() << "basedir" << patch->baseDir();

        connect( m_patch.data(), &IPatchSource::patchChanged, this, &PatchReviewPlugin::notifyPatchChanged );
    }
    QString finishText = i18nc("@action", "Finish Review");
    if( m_patch && !m_patch->finishReviewCustomText().isEmpty() )
      finishText = m_patch->finishReviewCustomText();
    m_finishReview->setText( finishText );
    m_finishReview->setEnabled( patch );

    notifyPatchChanged();
}

PatchReviewPlugin::PatchReviewPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : KDevelop::IPlugin(QStringLiteral("kdevpatchreview"), parent, metaData)
    , m_patch(nullptr)
    , m_factory(new PatchReviewToolViewFactory(this))
{
    setXMLFile( QStringLiteral("kdevpatchreview.rc") );

    connect( ICore::self()->documentController(), &IDocumentController::documentClosed, this, &PatchReviewPlugin::documentClosed );
    connect( ICore::self()->documentController(), &IDocumentController::textDocumentCreated, this, &PatchReviewPlugin::textDocumentCreated );
    connect( ICore::self()->documentController(), &IDocumentController::documentSaved, this, &PatchReviewPlugin::documentSaved );

    m_updateKompareTimer = new QTimer( this );
    m_updateKompareTimer->setSingleShot( true );
    m_updateKompareTimer->setInterval(500);
    connect( m_updateKompareTimer, &QTimer::timeout, this, &PatchReviewPlugin::updateKompareModel );

    m_finishReview = new QAction(i18nc("@action", "Finish Review"), this);
    m_finishReview->setIcon( QIcon::fromTheme( QStringLiteral("dialog-ok") ) );
    actionCollection()->setDefaultShortcut( m_finishReview, Qt::CTRL|Qt::Key_Return );
    actionCollection()->addAction(QStringLiteral("commit_or_finish_review"), m_finishReview);

    const auto allAreas = ICore::self()->uiController()->allAreas();
    for (Sublime::Area* area : allAreas) {
        if (area->objectName() == QLatin1String("review"))
            area->addAction(m_finishReview);
    }

    core()->uiController()->addToolView(i18nc("@title:window", "Patch Review"), m_factory, IUiController::None);

    areaChanged(ICore::self()->uiController()->activeArea());
}

void PatchReviewPlugin::documentClosed( IDocument* doc ) {
    removeHighlighting( doc->url() );
}

void PatchReviewPlugin::documentSaved( IDocument* doc ) {
    // Only update if the url is not the patch-file, because our call to
    // the reload() KTextEditor function also causes this signal,
    // which would lead to an endless update loop.
    // Also, don't automatically update local patch sources, because
    // they may correspond to static files which don't match any more
    // after an edit was done.
    if (m_patch && doc->url() != m_patch->file() && !qobject_cast<LocalPatchSource*>(m_patch.data())) {
        forceUpdate();
    }
}

void PatchReviewPlugin::textDocumentCreated( IDocument* doc ) {
    if (m_patch) {
        addHighlighting( doc->url(), doc );
    }
}

void PatchReviewPlugin::unload() {
    core()->uiController()->removeToolView( m_factory );

    KDevelop::IPlugin::unload();
}

void PatchReviewPlugin::areaChanged(Sublime::Area* area)
{
    bool reviewing = area->objectName() == QLatin1String("review");
    m_finishReview->setEnabled(reviewing);
    if(!reviewing) {
        closeReview();
    }
}

KDevelop::ContextMenuExtension PatchReviewPlugin::contextMenuExtension(KDevelop::Context* context, QWidget* parent)
{
    QList<QUrl> urls;

    if ( context->type() == KDevelop::Context::FileContext ) {
        auto* filectx = static_cast<KDevelop::FileContext*>(context);
        urls = filectx->urls();
    } else if ( context->type() == KDevelop::Context::ProjectItemContext ) {
        auto* projctx = static_cast<KDevelop::ProjectItemContext*>(context);
        const auto items = projctx->items();
        for (KDevelop::ProjectBaseItem* item : items) {
            if ( item->file() ) {
                urls << item->file()->path().toUrl();
            }
        }
    } else if ( context->type() == KDevelop::Context::EditorContext ) {
        auto* econtext = static_cast<KDevelop::EditorContext*>(context);
        urls << econtext->url();

        if (urls.constFirst().isEmpty()) {
            // This must be an Untitled document. The Review Patch action makes no sense for an unsaved document,
            // and triggering it causes an assertion failure in DocumentControllerPrivate::openDocumentInternal().
            // Do not add our context menu item to prevent this.
            urls.clear();
        }
    }

    if (urls.size() == 1) {
        auto* reviewAction = new QAction(QIcon::fromTheme(QStringLiteral("text-x-patch")),
                                         i18nc("@action:inmenu", "Review Patch"), parent);
        reviewAction->setData(QVariant(urls[0]));
        connect( reviewAction, &QAction::triggered, this, &PatchReviewPlugin::executeFileReviewAction );
        ContextMenuExtension cm;
        cm.addAction( KDevelop::ContextMenuExtension::VcsGroup, reviewAction );
        return cm;
    }

    return KDevelop::IPlugin::contextMenuExtension(context, parent);
}

void PatchReviewPlugin::executeFileReviewAction()
{
    auto* reviewAction = qobject_cast<QAction*>(sender());
    KDevelop::Path path(reviewAction->data().toUrl());
    auto* ps = new LocalPatchSource();
    ps->setFilename(path.toUrl());
    ps->setBaseDir(path.parent().toUrl());
    ps->setAlreadyApplied(true);
    ps->createWidget();
    startReview(ps, OpenAndRaise);
}

#include "patchreview.moc"
#include "moc_patchreview.cpp"
