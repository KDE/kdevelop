/***************************************************************************
 *   Copyright 2006-2007 Alexander Dymo <adymo@kdevelop.org>               *
 *   Copyright 2006 Andreas Pakulat <apaku@gmx.de>                         *
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
#include <QLayout>
#include <QAbstractItemView>
#include <QUrl>
#include <QtWidgets/QFileDialog>

#include <KI18n/KLocalizedString>
#include <KIOCore/KIO/StoredTransferJob>
#include <KIOFileWidgets/KFilePlacesModel>
#include <KParts/MainWindow>
#include <KXmlGui/KActionCollection>
#include <KWidgetsAddons/KMessageBox>
#include <KWidgetsAddons/KActionMenu>


#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>

#include "../openwith/iopenwith.h"

#include "kdevfilemanagerplugin.h"
#include "bookmarkhandler.h"
#include "debug.h"

Q_LOGGING_CATEGORY(PLUGIN_FILEMANAGER, "kdevplatform.plugins.filemanager")

FileManager::FileManager(KDevFileManagerPlugin *plugin, QWidget* parent)
    : QWidget(parent), m_plugin(plugin)
{
    setObjectName("FileManager");
    setWindowIcon(QIcon::fromTheme("system-file-manager"));
    setWindowTitle(i18n("Filesystem"));

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setMargin(0);
    l->setSpacing(0);
    KFilePlacesModel* model = new KFilePlacesModel( this );
    urlnav = new KUrlNavigator(model, QUrl::fromLocalFile(QDir::homePath()), this );
    connect(urlnav, SIGNAL(urlChanged(QUrl)), SLOT(gotoUrl(QUrl)));
    l->addWidget(urlnav);
    dirop = new KDirOperator(QUrl::fromLocalFile(QDir::homePath()), this);
    dirop->setView( KFile::Tree );
    dirop->setupMenu( KDirOperator::SortActions | KDirOperator::FileActions | KDirOperator::NavActions | KDirOperator::ViewActions );
    connect(dirop, SIGNAL(urlEntered(QUrl)), SLOT(updateNav(QUrl)));
    connect(dirop, SIGNAL(contextMenuAboutToShow(KFileItem,QMenu*)), SLOT(fillContextMenu(KFileItem,QMenu*)));
    l->addWidget(dirop);

    connect( dirop, SIGNAL(fileSelected(KFileItem)), this, SLOT(openFile(KFileItem)) );


    // includes some actions, but not hooked into the shortcut dialog atm
    m_actionCollection = new KActionCollection(this);
    m_actionCollection->addAssociatedWidget(this);

    setupActions();

    // Connect the bookmark handler
    connect(m_bookmarkHandler, SIGNAL(openUrl(QUrl)), this, SLOT(gotoUrl(QUrl)));
    connect(m_bookmarkHandler, SIGNAL(openUrl(QUrl)), this, SLOT(updateNav(QUrl)));
}

void FileManager::fillContextMenu(KFileItem item, QMenu* menu)
{
    foreach(QAction* a, contextActions){
        if(menu->actions().contains(a)){
            menu->removeAction(a);
        }
    }
    contextActions.clear();
    contextActions.append(menu->addSeparator());
    menu->addAction(newFileAction);
    contextActions.append(newFileAction);
    KDevelop::FileContext context(QList<QUrl>() << item.url());
    QList<KDevelop::ContextMenuExtension> extensions = KDevelop::ICore::self()->pluginController()->queryPluginsForContextMenuExtensions( &context );
    KDevelop::ContextMenuExtension::populateMenu(menu, extensions);
    QMenu* tmpMenu = new QMenu();
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
    KActionMenu *acmBookmarks = new KActionMenu(QIcon::fromTheme("bookmarks"), i18n("Bookmarks"), this);
    acmBookmarks->setDelayed(false);
    m_bookmarkHandler = new BookmarkHandler(this, acmBookmarks->menu());
    acmBookmarks->setShortcutContext(Qt::WidgetWithChildrenShortcut);

    QAction* action = new QAction(this);
    action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    action->setText(i18n("Current Document Directory"));
    action->setIcon(QIcon::fromTheme("dirsync"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(syncCurrentDocumentDirectory()));
    tbActions << (dirop->actionCollection()->action("back"));
    tbActions << (dirop->actionCollection()->action("up"));
    tbActions << (dirop->actionCollection()->action("home"));
    tbActions << (dirop->actionCollection()->action("forward"));
    tbActions << (dirop->actionCollection()->action("reload"));
    tbActions << acmBookmarks;
    tbActions << action;
    tbActions << (dirop->actionCollection()->action("sorting menu"));
    tbActions << (dirop->actionCollection()->action("show hidden"));

    newFileAction = new QAction(this);
    newFileAction->setText(i18n("New File..."));
    newFileAction->setIcon(QIcon::fromTheme("document-new"));
    connect(newFileAction, SIGNAL(triggered()), this, SLOT(createNewFile()));
}

void FileManager::createNewFile()
{
    QUrl destUrl = QFileDialog::getSaveFileUrl(KDevelop::ICore::self()->uiController()->activeMainWindow(), i18n("Create New File"));
    if (destUrl.isEmpty()) {
        return;
    }

    KJob* job = KIO::storedPut(QByteArray(), destUrl, -1);
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
