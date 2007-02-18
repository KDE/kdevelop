/***************************************************************************
 *   Copyright (C) 2006 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *   Copyright (C) 2006 by Andreas Pakulat                                 *
 *   apaku@gmx.de                                                          *
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
#include <QDirModel>
#include <QLayout>
#include <QTreeView>
#include <QTimer>
#include <QThread>

#include <kurl.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdirlister.h>
#include <ktoolbar.h>
#include <kaction.h>
#include <kstandardaction.h>

#include "icore.h"
#include "iuicontroller.h"

#include "kdevfilemanagerpart.h"
#include "drilldownview.h"
#include "kdevdirmodel.h"

class FileManagerPrivate
{
public:

    FileManager *m_manager;
    KDevDirModel *m_model;
    DrillDownView *m_view;
    KToolBar *toolBar;
    KDevFileManagerPart *m_part;

    FileManagerPrivate(FileManager *manager): m_manager(manager) {}

    void open(const QModelIndex &index)
    {
        if (!index.isValid())
            return;

        KFileItem *fileItem = m_model->itemForIndex(index);
        if (!fileItem)
            return;

        if (fileItem->isFile())
            openFile(fileItem);
        else if (fileItem->isDir())
            m_view->slideRight();
    }

    void openFile(KFileItem *fileItem)
    {
        if (!fileItem)
            return;

        m_part->core()->uiController()->openUrl(fileItem->url());
    }

    void init()
    {
        goHome();
    }

    void goUp()
    {
        m_model->goUp();
    }

    void goHome()
    {
        m_model->dirLister()->openUrl(KUrl::fromPath(QDir::homePath()));
    }
};

class ToolBarParent: public QWidget {
public:
    ToolBarParent(QWidget *parent): QWidget(parent), m_toolBar(0) {}
    void setToolBar(QToolBar *toolBar)
    {
        m_toolBar = toolBar;
    }

    virtual void resizeEvent(QResizeEvent *ev)
    {
        if (!m_toolBar)
            return QWidget::resizeEvent(ev);
        setMinimumHeight(m_toolBar->sizeHint().height());
        m_toolBar->resize(width(), height());
    }

private:
    QToolBar *m_toolBar;
};


FileManager::FileManager(KDevFileManagerPart *part, QWidget* parent)
    :QWidget(parent), d(new FileManagerPrivate(this))
{
    d->m_part = part;
    setObjectName("FileManager");
//     setWindowIcon(SmallIcon("kdevelop"));
    setWindowTitle(i18n("File Manager"));
    setWhatsThis(i18n("File Manager"));

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setMargin(0);
    l->setSpacing(0);

    ToolBarParent *tbp = new ToolBarParent(this);
    l->addWidget(tbp);

    d->toolBar = new KToolBar(tbp);
    d->toolBar->setMovable(false);
    d->toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    d->toolBar->setIconSize(QSize(16,16));
    d->toolBar->setContextMenuEnabled(false);
    tbp->setToolBar(d->toolBar);

    d->m_view = new DrillDownView(this);
    l->addWidget(d->m_view);

    d->m_model = new KDevDirModel(d->m_view);
    d->m_view->setModel(d->m_model);

    connect(d->m_view, SIGNAL(doubleClicked(const QModelIndex &)),
        this, SLOT(open(const QModelIndex &)));
    connect(d->m_view, SIGNAL(returnPressed(const QModelIndex &)),
        this, SLOT(open(const QModelIndex &)));
    connect(d->m_view, SIGNAL(tryToSlideLeft()),
        this, SLOT(goUp()));

    setupActions();

    QTimer::singleShot(0, this, SLOT(init()));
}

void FileManager::setupActions()
{
    KAction *action = KStandardAction::home(this, SLOT(goHome()), 0);
    d->toolBar->addAction(action);
}

#include "filemanager.moc"
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on;
