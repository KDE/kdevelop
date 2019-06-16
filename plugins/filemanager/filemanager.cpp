/***************************************************************************
 *   Copyright 2006-2007 Alexander Dymo <adymo@kdevelop.org>               *
 *   Copyright 2006 Andreas Pakulat <apaku@gmx.de>                         *
 *   Copyright 2016 Imran Tatriev <itatriev@gmail.com>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
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
#include "filemanager.h"

#include <QDir>
#include <QMenu>
#include <QVBoxLayout>
#include <QUrl>
#include <QFileDialog>

#include <KLocalizedString>
#include <KIO/StoredTransferJob>
#include <KFilePlacesModel>
#include <KParts/MainWindow>
#include <KActionCollection>
#include <KMessageBox>
#include <KActionMenu>
#include <KJobWidgets>
#include <KConfigGroup>

#include <interfaces/icore.h>
#include <interfaces/isession.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>

#include "../openwith/iopenwith.h"

#include "kdevfilemanagerplugin.h"
#include "bookmarkhandler.h"
#include "debug.h"

FileManager::FileManager(KDevFileManagerPlugin *plugin, QWidget* parent)
    : QWidget(parent), m_plugin(plugin)
{
    setObjectName(QStringLiteral("FileManager"));
    setWindowIcon(QIcon::fromTheme(QStringLiteral("folder-sync"), windowIcon()));
    setWindowTitle(i18n("File System"));

    KConfigGroup cg = KDevelop::ICore::self()->activeSession()->config()->group( "Filesystem" );

    auto *l = new QVBoxLayout(this);
    l->setMargin(0);
    l->setSpacing(0);
    auto* model = new KFilePlacesModel( this );
    urlnav = new KUrlNavigator(model, QUrl(cg.readEntry( "LastLocation", QUrl::fromLocalFile( QDir::homePath() ) )), this );

    connect(urlnav, &KUrlNavigator::urlChanged, this, &FileManager::gotoUrl);

    l->addWidget(urlnav);
    dirop = new KDirOperator( urlnav->locationUrl(), this);
    dirop->setView( KFile::Tree );
    dirop->setupMenu( KDirOperator::SortActions | KDirOperator::FileActions | KDirOperator::NavActions | KDirOperator::ViewActions );
    connect(dirop, &KDirOperator::urlEntered, this, &FileManager::updateNav);
    connect(dirop, &KDirOperator::contextMenuAboutToShow, this, &FileManager::fillContextMenu);
    l->addWidget(dirop);

    connect( dirop, &KDirOperator::fileSelected, this, &FileManager::openFile );

    setFocusProxy(dirop);

    // includes some actions, but not hooked into the shortcut dialog atm
    m_actionCollection = new KActionCollection(this);
    m_actionCollection->addAssociatedWidget(this);

    setupActions();

    // Connect the bookmark handler
    connect(m_bookmarkHandler, &BookmarkHandler::openUrl, this, &FileManager::gotoUrl);
    connect(m_bookmarkHandler, &BookmarkHandler::openUrl, this, &FileManager::updateNav);
}

FileManager::~FileManager()
{
    KConfigGroup cg = KDevelop::ICore::self()->activeSession()->config()->group( "Filesystem" );
    cg.writeEntry( "LastLocation", urlnav->locationUrl() );
    cg.sync();
}

void FileManager::fillContextMenu(const KFileItem& item, QMenu* menu)
{
    for (QAction* a : qAsConst(contextActions)) {
        if(menu->actions().contains(a)){
            menu->removeAction(a);
        }
    }
    contextActions.clear();
    contextActions.append(menu->addSeparator());
    menu->addAction(newFileAction);
    contextActions.append(newFileAction);
    KDevelop::FileContext context(QList<QUrl>() << item.url());
    QList<KDevelop::ContextMenuExtension> extensions = KDevelop::ICore::self()->pluginController()->queryPluginsForContextMenuExtensions(&context, menu);
    KDevelop::ContextMenuExtension::populateMenu(menu, extensions);
    auto* tmpMenu = new QMenu();
    KDevelop::ContextMenuExtension::populateMenu(tmpMenu, extensions);
    contextActions.append(tmpMenu->actions());
    delete tmpMenu;
}

void FileManager::openFile(const KFileItem& file)
{
    KDevelop::IOpenWith::openFiles(QList<QUrl>() << file.url());
}


void FileManager::gotoUrl( const QUrl& url )
{
     dirop->setUrl( url, true );
}

void FileManager::updateNav( const QUrl& url )
{
    urlnav->setLocationUrl( url );
}

void FileManager::setupActions()
{
    KActionMenu *acmBookmarks = new KActionMenu(QIcon::fromTheme(QStringLiteral("bookmarks")), i18n("Bookmarks"), this);
    acmBookmarks->setDelayed(false);
    m_bookmarkHandler = new BookmarkHandler(this, acmBookmarks->menu());
    acmBookmarks->setShortcutContext(Qt::WidgetWithChildrenShortcut);

    auto* action = new QAction(this);
    action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    action->setText(i18n("Current Document Directory"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("dirsync")));
    connect(action, &QAction::triggered, this, &FileManager::syncCurrentDocumentDirectory);
    auto* diropActionCollection = dirop->actionCollection();
    tbActions = {
        diropActionCollection->action(QStringLiteral("back")),
        diropActionCollection->action(QStringLiteral("up")),
        diropActionCollection->action(QStringLiteral("home")),
        diropActionCollection->action(QStringLiteral("forward")),
        diropActionCollection->action(QStringLiteral("reload")),
        acmBookmarks,
        action,
        diropActionCollection->action(QStringLiteral("sorting menu")),
        diropActionCollection->action(QStringLiteral("show hidden")),
    };

    newFileAction = new QAction(this);
    newFileAction->setText(i18n("New File..."));
    newFileAction->setIcon(QIcon::fromTheme(QStringLiteral("document-new")));
    connect(newFileAction, &QAction::triggered, this, &FileManager::createNewFile);
}

void FileManager::createNewFile()
{
    QUrl destUrl = QFileDialog::getSaveFileUrl(KDevelop::ICore::self()->uiController()->activeMainWindow(), i18n("Create New File"));
    if (destUrl.isEmpty()) {
        return;
    }

    KJob* job = KIO::storedPut(QByteArray(), destUrl, -1);
    KJobWidgets::setWindow(job, this);
    connect(job, &KJob::result, this, &FileManager::fileCreated);
}

void FileManager::fileCreated(KJob* job)
{
    auto transferJob = qobject_cast<KIO::StoredTransferJob*>(job); Q_ASSERT(transferJob);
    if (!transferJob->error()) {
        KDevelop::ICore::self()->documentController()->openDocument( transferJob->url() );
    } else {
        KMessageBox::error(KDevelop::ICore::self()->uiController()->activeMainWindow(), i18n("Unable to create file '%1'", transferJob->url().toDisplayString(QUrl::PreferLocalFile)));
    }
}

void FileManager::syncCurrentDocumentDirectory()
{
    if( KDevelop::IDocument* activeDoc = KDevelop::ICore::self()->documentController()->activeDocument() )
        updateNav( activeDoc->url().adjusted(QUrl::RemoveFilename) );
}

QList<QAction*> FileManager::toolBarActions() const
{
    return tbActions;
}

KActionCollection* FileManager::actionCollection() const
{
    return m_actionCollection;
}

KDirOperator* FileManager::dirOperator() const
{
    return dirop;
}

KDevFileManagerPlugin* FileManager::plugin() const
{
    return m_plugin;
}

#include "moc_filemanager.cpp"
