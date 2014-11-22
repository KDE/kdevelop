/* This file is part of KDevelop
 * Copyright 2011 Mathieu Lornac <mathieu.lornac@gmail.com>
 * Copyright 2011 Damien Coppel <damien.coppel@gmail.com>
 * Copyright 2011 Lionel Duc <lionel.data@gmail.com>
 * Copyright 2013 Christoph Thielecke <crissi99@gmx.de>

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

#include <KTextEditor/Document>
#include <KTextEditor/Editor>
#include <KTextEditor/MarkInterface>
#include <QAbstractItemModel>
#include <QDir>
#include <QUrl>

#include <kconfig.h>
#include <kconfiggroup.h>

#include "debug.h"
#include "marks.h"
#include "cppcheckmodel.h"
#include "cppcheck_file_model.h"
#include "cppcheck_severity_model.h"
#include "cppcheck_file_item.h"
#include "cppcheck_severity_item.h"
#include "plugin.h"
#include "modelwrapper.h"
#include "cppcheckview.h"

namespace cppcheck
{

Marks::Marks(cppcheck::Plugin *plugin)
    : m_plugin(plugin)
    , m_model(0)
{
    connect(plugin, SIGNAL(newModel(cppcheck::Model*)), this, SLOT(newModel(cppcheck::Model*)));
}

Marks::~Marks()
{
}


void Marks::newModel(cppcheck::Model* model)
{
    m_model = model;
    connect(m_model->getModelWrapper(), SIGNAL(modelChanged()),
            this, SLOT(modelChanged()));
}

void Marks::modelChanged()
{
    // parse model to display errors in the editor

    auto editor = KTextEditor::Editor::instance();
    QList<KTextEditor::Document*> docList = editor->documents();
//     qCDebug(KDEV_CPPCHECK) << "docList: ";
//     for (int i = 0; i < docList.size(); ++i)
//         qCDebug(KDEV_CPPCHECK) << "\t" << docList.at(i)->documentName() << ", (" << docList.at(i)->url() << ")";
    
    for (int i = 0; i < docList.size(); i++)
        if (KTextEditor::MarkInterface *iface = qobject_cast<KTextEditor::MarkInterface*>(docList.at(i)))
            iface->clearMarks();
    // errors

    KConfig config("kdevcppcheckrc");
    KConfigGroup grp = config.group("cppcheck");

    int OutputViewMode = grp.readEntry("OutputViewMode", 0);

    QAbstractItemModel *itemModel = m_model->getQAbstractItemModel();
    int numRows = itemModel->rowCount();
    for (int row = 0; row < numRows; row++) {
        int childCount = 1;
        QString FileName = "";
        int LineNumber = 0;
        QModelIndex myIndex = QModelIndex();
        if (OutputViewMode == cppcheck::CppcheckView::flatOutputMode) {
            FileName = itemModel->index(row, CppcheckModel::ProjectPath).data().toString() + itemModel->index(row, CppcheckModel::ErrorFile).data().toString();
            LineNumber = (itemModel->index(row, CppcheckModel::ErrorLine).data()).toInt();
        }
        else if (OutputViewMode == cppcheck::CppcheckView::groupedByFileOutputMode ) {
            myIndex = itemModel->index(row, 0);
            CppcheckFileItem *item = static_cast<CppcheckFileItem*>(myIndex.internalPointer());
            if (itemModel->hasChildren()) {
                childCount = itemModel->rowCount(myIndex);
                FileName = item->ProjectPath + item->ErrorFile;
                qCDebug(KDEV_CPPCHECK) << "child count of " << FileName << ": " << childCount;
                if (FileName == "")
                    continue;
            }
        }
        else if (OutputViewMode == cppcheck::CppcheckView::groupedBySeverityOutputMode ) {
            myIndex = itemModel->index(row, 0);
            if (itemModel->hasChildren()) {
                childCount = itemModel->rowCount(myIndex);
                CppcheckSeverityItem *item = static_cast<CppcheckSeverityItem*>(myIndex.internalPointer());
                qCDebug(KDEV_CPPCHECK) << "child count of " << item->Severity << ": " << childCount;
            }
        }

        for (int x=0; x < childCount; x++) {
            if (OutputViewMode == cppcheck::CppcheckView::groupedByFileOutputMode) {
                CppcheckFileItem *parentItem = static_cast<CppcheckFileItem*>(myIndex.internalPointer());
                CppcheckFileItem *childitem = parentItem->child(x);
                if (childitem)
                    LineNumber = childitem->ErrorLine;
            }
            else if (OutputViewMode == cppcheck::CppcheckView::groupedBySeverityOutputMode) {
                CppcheckSeverityItem *parentItem = static_cast<CppcheckSeverityItem*>(myIndex.internalPointer());
                CppcheckSeverityItem *childitem = parentItem->child(x);
                if (childitem)
                {
                    FileName = childitem->ProjectPath + childitem->ErrorFile;
                    LineNumber = childitem->ErrorLine;
                }
            }
            //qCDebug(KDEV_CPPCHECK) << "row[" << row << "]: filename: " << FileName <<  ", line: " << LineNumber;
            if (FileName.isEmpty() )
                continue;
            for (int i = 0; i < docList.size(); ++i) {
                    //qCDebug(KDEV_CPPCHECK) << "doc: " << docList.at(i)->url() << " <=> " << KUrl(FileName);
                if (docList.at(i)->url() == QUrl::fromLocalFile(FileName) ) {
                    if (KTextEditor::MarkInterface *iface = qobject_cast<KTextEditor::MarkInterface*>(docList.at(i))) {
                        iface->addMark(LineNumber - 1, KTextEditor::MarkInterface::markType07);
                        qCDebug(KDEV_CPPCHECK) << "adding mark at: " << docList.at(i)->url() << ":" << LineNumber;
                    }
                }
            }
        }
    }
}

}
