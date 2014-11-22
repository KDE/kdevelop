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
#include <QHeaderView>

#include <kio/statjob.h>
#include <KTextEditor/Document>
#include <KTextEditor/MovingInterface>

#include <KConfig>
#include <KConfigGroup>
#include <KJobUiDelegate>
#include <KJobWidgets>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>

#include "debug.h"
#include "cppcheckitemsimpl.h"
#include "models/cppcheckmodel.h"
#include "models/cppcheck_file_item.h"
#include "models/cppcheck_severity_item.h"

namespace {

bool exists(const QUrl& url, QWidget* parent)
{
    auto job = KIO::stat(url, KIO::StatJob::SourceSide, 0);
    KJobWidgets::setWindow(job, parent);
    return job->exec();
}

}

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
        QUrl doc = frame->url();
        if (doc.isValid() && exists(doc, qApp->activeWindow())) {
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
    if (OutputViewMode == cppcheck::CppcheckView::flatOutputMode)
        LineNumber = index.model()->index(row, CppcheckModel::ErrorLine).data().toInt();
    else if (OutputViewMode == cppcheck::CppcheckView::groupedByFileOutputMode) {
        CppcheckFileItem *item = static_cast<CppcheckFileItem*>(index.internalPointer());
        if (!item->isChild())
            return;

        FileName = item->ProjectPath + QDir::separator() +  item->ErrorFile;
        LineNumber = item->ErrorLine;
        qCDebug(KDEV_CPPCHECK) << "(row: " << row << ") data: " << FileName << "=> " << QString().setNum(LineNumber);
    }
    else if (OutputViewMode == cppcheck::CppcheckView::groupedBySeverityOutputMode) {
        CppcheckSeverityItem *item = static_cast<CppcheckSeverityItem*>(index.internalPointer());
        if (!item->isChild())
            return;

        FileName = item->ProjectPath + QDir::separator() +  item->ErrorFile;
        LineNumber = item->ErrorLine;
        qCDebug(KDEV_CPPCHECK) << "(row: " << row << ") data: " << FileName << "=> " << QString().setNum(LineNumber);
    }

    qCDebug(KDEV_CPPCHECK) << "double clicked: (row: " << row << ") " << ClickedCellContent << "=> " << ProjectPath + FileName << ":" << LineNumber ;
    if (LineNumber > -1) {
        // go there
        // TODO: Check
        QUrl doc = QUrl(ProjectPath + FileName);
        if (doc.isValid() && exists(doc, qApp->activeWindow())) {

            // check ifts open (all files check)
            if (KDevelop::ICore::self()->documentController()->documentForUrl(doc) == 0) {
                // not open
                qCDebug(KDEV_CPPCHECK) << "file not open, open it";
                KDevelop::ICore::self()->documentController()->openDocument(doc, KTextEditor::Cursor(qMax(0, LineNumber - 1), 0));
            } else {
                qCDebug(KDEV_CPPCHECK) << "file already open";
                // open
                KDevelop::ICore::self()->documentController()->openDocument(doc, KTextEditor::Cursor(qMax(0, LineNumber - 1), 0));
            }
        }
    }
}

}
