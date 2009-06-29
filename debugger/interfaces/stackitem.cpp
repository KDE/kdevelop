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

#include <KIcon>
#include <KLocalizedString>

#include "stackmodel.h"
#include "../../interfaces/icore.h"
#include "../../interfaces/idocumentcontroller.h"
#include "../../interfaces/iprojectcontroller.h"
#include "../../interfaces/iproject.h"

KDevelop::FrameItem::FrameItem(KDevelop::FramesModel* model)
    : TreeItem(model, model->root()), mModel(model)
{ Q_ASSERT(model->root()); }

int KDevelop::FrameItem::id() const
{
    return mId;
}

void KDevelop::FrameItem::setInformation(int id, const QString& name, const QString& location, int line)
{
    mId=id;
    mLocation=qMakePair(location, line);
    QString l = ICore::self()->projectController()->prettyFileName(KUrl(location), IProjectController::FormatPlain);
    setData(QVector<QVariant>() << QString::number(id)
                                << name
                                << QString(l+':'+QString::number(line)));
}

KDevelop::FramesModel* KDevelop::FrameItem::framesModel()
{
    Q_ASSERT(mModel);
    return mModel;
}

void KDevelop::FrameItem::clicked()
{
    ICore::self()->documentController()->openDocument(mLocation.first, KTextEditor::Cursor(mLocation.second-1, 0));
}

QVariant KDevelop::FrameItem::icon(int column) const
{
    if(column==2) {
        KMimeType::Ptr p=KMimeType::findByUrl(mLocation.first);
        return KIcon(p->iconName());
    }
    return QVariant();
}

KDevelop::FramesModel* KDevelop::ThreadItem::framesModel()
{
    Q_ASSERT(mFramesModel && mFramesModel->rowCount()>=0);
    return mFramesModel;
}

int KDevelop::ThreadItem::id() const
{
    return mId;
}

KDevelop::StackModel* KDevelop::ThreadItem::stackModel()
{
    return mStackModel;
}

KDevelop::ThreadItem::ThreadItem(KDevelop::StackModel* model)
    : TreeItem(model, model->root()), mFramesModel(0), mStackModel(model)
{
    Q_ASSERT(model->root());
    mFramesModel=new FramesModel(mStackModel, this);
}

void KDevelop::ThreadItem::setInformation(int threadNumber, const QString& functionName)
{
    mId=threadNumber;
    setData(QVector<QVariant>() << i18nc("#thread-id at function-name or address", "#%1 at %2", threadNumber, functionName));
}

#include "moc_stackitem.cpp"
