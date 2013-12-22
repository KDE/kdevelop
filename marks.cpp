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

#include <kconfig.h>
#include <kconfiggroup.h>

#include "marks.h"
#include "cppcheckmodel.h"
#include "cppcheck_file_model.h"
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
    KTextEditor::Editor* editor = KTextEditor::editor("katepart");
    QList<KTextEditor::Document*> docList = editor->documents();
//     kDebug() << "docList: ";
//     for (int i = 0; i < docList.size(); ++i)
//         kDebug() << "\t" << docList.at(i)->documentName() << ", (" << docList.at(i)->url() << ")";
    
    for (int i = 0; i < docList.size(); ++i)
        if (KTextEditor::MarkInterface *iface = qobject_cast<KTextEditor::MarkInterface*>(docList.at(i)))
            iface->clearMarks();
    // errors

    KConfig config("kdevcppcheckrc");
    KConfigGroup grp = config.group("cppcheck");

    int OutputViewMode = grp.readEntry("OutputViewMode", 0);

    QAbstractItemModel *itemModel = m_model->getQAbstractItemModel();
    int numRows = itemModel->rowCount();
    for (int row = 0; row < numRows; ++row) {
        int childCount = 1;
        QString FileName = "";
        int LineNumber = 0;
        QModelIndex myIndex = QModelIndex();
        if (OutputViewMode == cppcheck::CppcheckView::flatOutputMode) {
         FileName = itemModel->index(row, CppcheckModel::ProjectPath).data().toString() + itemModel->index(row, CppcheckModel::ErrorFile).data().toString();
         LineNumber = (itemModel->index(row, CppcheckModel::ErrorLine).data()).toInt();
        }
        if (OutputViewMode == cppcheck::CppcheckView::groupedByFileOutputMode ) {
            myIndex = itemModel->index(row, CppcheckModel::ProjectPath);
            if (itemModel->hasChildren()) {
                childCount = itemModel->rowCount(myIndex);
                kDebug() << "child count of " << FileName << ": " << childCount;
            }
        }

        for (int x=0; x < childCount; x++) {

            if (OutputViewMode == cppcheck::CppcheckView::groupedByFileOutputMode) {
                FileName = itemModel->index(x, CppcheckFileItem::ColumnProjectPath, myIndex).data().toString() + itemModel->index(row, CppcheckFileItem::ColumnErrorFile).data().toString();
                LineNumber = (itemModel->index(x, CppcheckFileItem::ColumnErrorFile, myIndex).data()).toInt();
            }
            //kDebug() << "row[" << row << "]: filename: " << FileName <<  ", line: " << LineNumber;
            if (FileName.isEmpty() )
                continue;
            for (int i = 0; i < docList.size(); ++i) {
                    //kDebug() << "doc: " << docList.at(i)->url() << " <=> " << KUrl(FileName);
                if (docList.at(i)->url() == KUrl(FileName) ) {
                    if (KTextEditor::MarkInterface *iface = qobject_cast<KTextEditor::MarkInterface*>(docList.at(i))) {
                        iface->addMark(LineNumber - 1, KTextEditor::MarkInterface::markType07);
                        kDebug() << "adding mark at: " << docList.at(i)->url() << ":" << LineNumber;
                    }
                }
            }
        }
    }
}

}

#include "marks.moc"
