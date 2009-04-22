/*
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "stackitem.h"
#include "stackmodel.h"
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <KIcon>

KDevelop::StackItem::StackItem(KDevelop::StackModel* model, KDevelop::TreeItem* parent, const QString& prefix)
    : TreeItem(model, parent), mPrefix(prefix), mModel(model)
{}

int KDevelop::StackItem::id() const
{
    return mId;
}

void KDevelop::StackItem::setInformation(int id, const QString& name, const QPair< QString, int >& location)
{
    mId=id;
    mLocation=location;
    setData(QVector<QVariant>() << (mPrefix+QString::number(id))
                                << name
                                << QString(location.first+':'+QString::number(location.second)));
}

KDevelop::StackModel* KDevelop::StackItem::stackModel()
{
    return mModel;
}

void KDevelop::StackItem::clicked()
{
    ICore::self()->documentController()->openDocument(mLocation.first, KTextEditor::Cursor(mLocation.second-1, 0));
}

QVariant KDevelop::StackItem::icon(int column) const
{
    if(column==2) {
        KMimeType::Ptr p=KMimeType::findByUrl(mLocation.first);
        return KIcon(p->iconName());
    }
    return QVariant();
}
