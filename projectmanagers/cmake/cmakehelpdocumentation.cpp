/* KDevelop CMake Support
 *
 * Copyright 2009 Aleix Pol <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "cmakehelpdocumentation.h"
#include <QTreeView>
#include <KLocalizedString>
#include "cmakedoc.h"
#include "icmakedocumentation.h"
#include "cmakedocumentation.h"
#include <QHeaderView>
#include <interfaces/icore.h>
#include <interfaces/idocumentationcontroller.h>

static QString modules [] = {
    i18n("Commands"), i18n("Variables"), i18n("Modules"), i18n("Properties"), i18n("Policies")
};

CMakeContentsModel::CMakeContentsModel(QObject* parent)
    : QAbstractItemModel(parent)
{}

QString CMakeHomeDocumentation::name() const
{
    return i18n("CMake Content Page");
}

KDevelop::IDocumentationProvider* CMakeHomeDocumentation::provider() const
{
    return CMakeDoc::s_provider;
}

QWidget* CMakeHomeDocumentation::documentationWidget(KDevelop::DocumentationFindWidget*, QWidget* parent)
{
    QTreeView* contents=new QTreeView(parent);
    contents->header()->setVisible(false);

    CMakeContentsModel* model=new CMakeContentsModel(contents);
    contents->setModel(model);
    QObject::connect(contents, &QTreeView::clicked, model, &CMakeContentsModel::showItem);
    return contents;
}

//Model methods implementation
QModelIndex CMakeContentsModel::parent(const QModelIndex& child) const
{
    if(child.isValid() && child.column()==0 && int(child.internalId())>=0)
        return createIndex(child.internalId(),0, -1);
    return QModelIndex();
}

QModelIndex CMakeContentsModel::index(int row, int column, const QModelIndex& parent) const
{
    if(row<0 || column!=0)
        return QModelIndex();
    if(!parent.isValid() && row==ICMakeDocumentation::EOType)
        return QModelIndex();

    return createIndex(row,column, int(parent.isValid() ? parent.row() : -1));
}

int CMakeContentsModel::rowCount(const QModelIndex& parent) const
{
    if(!parent.isValid())
        return ICMakeDocumentation::EOType;
    else if(int(parent.internalId())<0) {
        int ss=CMakeDoc::s_provider->names((ICMakeDocumentation::Type) parent.row()).size();
        return ss;
    }
    return 0;
}

QVariant CMakeContentsModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid()) {
        if(role==Qt::DisplayRole) {
            int internal(index.internalId());
            if(internal>=0)
                return CMakeDoc::s_provider->names((ICMakeDocumentation::Type) internal).at(index.row());
            else
                return modules[index.row()];
        }
    }
    return QVariant();
}

void CMakeContentsModel::showItem(const QModelIndex& idx)
{
    if(idx.isValid() && int(idx.internalId())>=0) {
        QString desc=CMakeDoc::s_provider->descriptionForIdentifier(idx.data().toString(),
                                                                    (ICMakeDocumentation::Type) idx.parent().row());
        CMakeDoc::Ptr doc(new CMakeDoc(idx.data().toString(), desc));

        KDevelop::ICore::self()->documentationController()->showDocumentation(doc);
    }
}
