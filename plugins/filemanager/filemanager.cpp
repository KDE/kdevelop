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
// #include <kdirmodel.h>
#include <kdirlister.h>

#include "icore.h"
#include "iuicontroller.h"

#include "kdevfilemanagerpart.h"
#include "drilldownview.h"
#include "kdevdirmodel.h"

class FileManagerPrivate
{
public:

    KDirModel *m_model;
    DrillDownView *m_view;
    KDevFileManagerPart *m_part;
    void open(const QModelIndex &index)
    {
        if (!index.isValid())
            return;

        KFileItem *fileItem = m_model->itemForIndex(index);
        if (!fileItem)
            return;

        if (fileItem->isFile())
            openFile(fileItem);
    }

    void openFile(KFileItem *fileItem)
    {
        if (!fileItem)
            return;

        m_part->core()->uiController()->openUrl(fileItem->url());
    }

    void init()
    {
        m_model->dirLister()->openUrl(KUrl::fromPath(QDir::rootPath()));
    }
};



FileManager::FileManager(KDevFileManagerPart *part, QWidget* parent)
    :QWidget(parent), d(new FileManagerPrivate)
{
    d->m_part = part;
    setObjectName("FileManager");
//     setWindowIcon(SmallIcon("kdevelop"));
    setWindowTitle(i18n("File Manager"));
    setWhatsThis(i18n("File Manager"));

    QHBoxLayout *l = new QHBoxLayout(this);
    l->setMargin(0);
    l->setSpacing(0);

    d->m_view = new DrillDownView(this);
    l->addWidget(d->m_view);

    d->m_model = new KDevDirModel(d->m_view);
    d->m_view->setModel(d->m_model);

    connect(d->m_view, SIGNAL(doubleClicked(const QModelIndex &)),
        this, SLOT(open(const QModelIndex &)));
    connect(d->m_view, SIGNAL(returnPressed(const QModelIndex &)),
        this, SLOT(open(const QModelIndex &)));

    QTimer::singleShot(0, this, SLOT(init()));
}

#include "filemanager.moc"
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
