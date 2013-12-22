/* This file is part of KDevelop
 *  Copyright 2013 Christoph Thielecke <crissi99@gmx.de>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public
 License as published by the Free Software Foundation; either
 version 2 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; see the file COPYING.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.

*/

#include "cppcheckview.h"

#include <QApplication>
#include <QDir>

#include <KIO/NetAccess>
#include <KTextEditor/Document>
#include <KTextEditor/MovingInterface>

#include <KDebug>
#include <kconfiggroup.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>

#include <QHeaderView>

#include "cppcheckitemsimpl.h"
#include "models/cppcheckmodel.h"
#include "models/cppcheck_file_item.h"

namespace cppcheck
{       
CppcheckView::CppcheckView()
{
    connect(this, SIGNAL(activated(QModelIndex)), SLOT(openDocument(QModelIndex)));
    connect(this, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(doubleClicked(const QModelIndex&)));
}

CppcheckView::~CppcheckView()
{
}

void CppcheckView::setModel(cppcheck::Model* m)
{
    QTreeView::setModel(m->getQAbstractItemModel());
    header()->setResizeMode(QHeaderView::ResizeToContents);
}


cppcheck::Model* CppcheckView::model(void)
{
    return dynamic_cast<cppcheck::Model*>(QTreeView::model());
}

void CppcheckView::openDocument(const QModelIndex& index)
{
    if (cppcheck::CppcheckFrame* frame = dynamic_cast<cppcheck::CppcheckFrame*>(static_cast<cppcheck::CppcheckModel*>(model())->itemForIndex(index))) {
        KUrl doc = frame->url();
        if (doc.isValid() && KIO::NetAccess::exists(doc, KIO::NetAccess::SourceSide, qApp->activeWindow())) {
            KDevelop::ICore::self()->documentController()->openDocument(doc, KTextEditor::Cursor(qMax(0, frame->line - 1), 0));
        }
    }
}

void CppcheckView::doubleClicked(const QModelIndex& index)
{
    int row = index.row();
    QString ClickedCellContent =  index.data().toString();
    QString ProjectPath = index.model()->index(row, CppcheckModel::ProjectPath).data().toString();
    QString FileName = index.model()->index(row, CppcheckModel::ErrorFile).data().toString();
    int LineNumber = 0;

    KConfig config("kdevcppcheckrc");
    KConfigGroup grp = config.group("cppcheck");

    int OutputViewMode = grp.readEntry("OutputViewMode", 0);
    // FIXME
    QString ModelName = "CppcheckModel";
    if (OutputViewMode == cppcheck::CppcheckView::flatOutputMode)
        LineNumber = index.model()->index(row, CppcheckModel::ErrorLine).data().toInt();
    else if (OutputViewMode == cppcheck::CppcheckView::groupedByFileOutputMode) {
        QModelIndex parentIndex = index.parent();
        FileName = index.model()->index(row, CppcheckFileItem::ColumnProjectPath, parentIndex).data().toString() + QDir::separator() +  parentIndex.model()->index(parentIndex.row(), CppcheckModel::ErrorFile).data().toString();
        LineNumber = index.model()->index(row, CppcheckFileItem::ColumnErrorFile, parentIndex).data().toInt();
        kDebug() << "(row: " << row << ") data: " << parentIndex.model()->index(parentIndex.row(), CppcheckModel::ErrorFile).data().toString() << "=> " << index.model()->index(row, CppcheckModel::ProjectPath).data().toString() ;
        kDebug() << "(row: " << row << ") data: " << index.model()->index(row, CppcheckModel::ErrorFile).data().toString() << "=> " << index.model()->index(row, CppcheckModel::ProjectPath).data().toString() ;
    }

    kDebug() << "double clicked: (row: " << row << ") " << ClickedCellContent << "=> " << ProjectPath + FileName << ":" << LineNumber ;
    if (LineNumber > -1) {
        // go there
        KUrl doc = KUrl(ProjectPath + FileName);
        if (doc.isValid() && KIO::NetAccess::exists(doc, KIO::NetAccess::SourceSide, qApp->activeWindow())) {

            // check ifts open (all files check)
            if (KDevelop::ICore::self()->documentController()->documentForUrl(doc) == 0) {
                // not open
                kDebug() << "file not open, open it";
                KDevelop::ICore::self()->documentController()->openDocument(doc, KTextEditor::Cursor(qMax(0, LineNumber - 1), 0));
            } else {
                kDebug() << "file already open";
                // open
                KDevelop::ICore::self()->documentController()->openDocument(doc, KTextEditor::Cursor(qMax(0, LineNumber - 1), 0));
            }
        }
    }
}

}

#include "cppcheckview.moc"
