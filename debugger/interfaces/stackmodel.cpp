/*
   Copyright 2009 Niko Sams <niko.sams@gmail.com>

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

#include "stackmodel.h"
#include "stackitem.h"
#include <KLocalizedString>
#include <KDebug>

using namespace KDevelop;

KDevelop::StackModel::StackModel(QObject* parent)
    : TreeModel(QVector<QString>() << i18n("Frames"), parent), m_autoUpdate(false)
{}

bool KDevelop::StackModel::hasThreads() const
{
    return rowCount()>0;
}

namespace KDevelop
{
class DumbItem : public TreeItem
{
    public:
        DumbItem(FramesModel* model, TreeItem* parent) : TreeItem(model, parent), mModel(model) {}
        
        void addChild(TreeItem* it) { appendChild(it); }
        void setHasMoreItems(bool b) { setHasMore(b); }
        virtual void fetchMoreChildren() { mModel->moreItems(); }
        bool hasMore() const { return TreeItem::hasMore(); }
        
        FramesModel *mModel;
        using KDevelop::TreeItem::clear;
};
}

KDevelop::FramesModel::FramesModel(QObject* parent, ThreadItem* thread)
    : TreeModel(QVector<QString>() << i18n("Depth") << i18n("Function") << i18n("Source"), parent), mThread(thread)
{
    mRoot=new DumbItem(this, 0);
    QTimer::singleShot(0, this, SLOT(initialize()));
}

void KDevelop::FramesModel::initialize()
{
    kDebug() << "dodongoooooooooooo";
    setRootItem(mRoot);
}

void KDevelop::FramesModel::addFrame(KDevelop::FrameItem* f)
{
    mRoot->addChild(f);
}

void KDevelop::FramesModel::setHasMoreFrames(bool m)
{
    mRoot->setHasMoreItems(m);
}

void KDevelop::StackModel::setAutoUpdate(bool autoUpdate) {
    m_autoUpdate = autoUpdate;
    if (autoUpdate)
        update();
}

KDevelop::FramesModel* KDevelop::StackModel::modelForThread(int id)
{
    Q_ASSERT(id<rowCount());
    FramesModel* f=static_cast<ThreadItem*>(itemForIndex(index(id,0)))->framesModel();
    Q_ASSERT(f);
    return f;
}

void KDevelop::FramesModel::moreItems()
{
    mThread->fetchMoreChildren();
}

int KDevelop::FramesModel::framesCount() const
{
    return rowCount()-mRoot->hasMore();
}

void KDevelop::FramesModel::removeAll() const
{
    mRoot->clear();
}

#include "stackmodel.moc"
