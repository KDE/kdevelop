/***************************************************************************
 *   Copyright (C) 2006 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
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

#include <kurl.h>
#include <klocale.h>
#include <kiconloader.h>
// #include <kdirmodel.h>
#include <kdirlister.h>

#include "kdevcore.h"
#include "kdevdocumentcontroller.h"

#include "kdevfilemanagerpart.h"
#include "drilldownview.h"
#include "kdevdirmodel.h"

FileManager::FileManager(KDevFileManagerPart *part)
    :QWidget(0), m_part(part)
{
    setObjectName("FileManager");
//     setWindowIcon(SmallIcon("kdevelop"));
    setWindowTitle(i18n("File Manager"));
    setWhatsThis(i18n("File Manager"));

    QHBoxLayout *l = new QHBoxLayout(this);
    l->setMargin(0);
    l->setSpacing(0);

    m_view = new DrillDownView(this);
    l->addWidget(m_view);

    init();
}

void FileManager::init()
{
    m_model = new KDevDirModel(this);
    m_model->dirLister()->openUrl(KUrl::fromPath("/"));
    m_view->setModel(m_model);

    connect(m_view, SIGNAL(doubleClicked(const QModelIndex &)),
        this, SLOT(open(const QModelIndex &)));
    connect(m_view, SIGNAL(returnPressed(const QModelIndex &)),
        this, SLOT(open(const QModelIndex &)));
}

void FileManager::open(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    KFileItem *fileItem = m_model->itemForIndex(index);
    if (!fileItem)
        return;

    if (fileItem->isFile())
        openFile(fileItem);
}

void FileManager::openFile(KFileItem *fileItem)
{
    if (!fileItem)
        return;

    KDevCore::documentController()->editDocument(fileItem->url());
}

#include "filemanager.moc"
