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

#ifndef KDEV_STACKMODEL_H
#define KDEV_STACKMODEL_H

#include "../util/treemodel.h"
#include "../debuggerexport.h"

namespace KDevelop
{
class ThreadItem;
class FrameItem;
class DumbItem;

class KDEVPLATFORMDEBUGGER_EXPORT FramesModel : public KDevelop::TreeModel
{
    Q_OBJECT
    public:
        FramesModel(QObject *parent, ThreadItem* thread);
        
        void addFrame(FrameItem* );
        void setHasMoreFrames(bool);
        void moreItems();
        int framesCount() const;
        void removeAll() const;
    private slots:
        void initialize(); 
    private:
        DumbItem* mRoot;
        ThreadItem* mThread;
};

class KDEVPLATFORMDEBUGGER_EXPORT StackModel : public TreeModel
{
    Q_OBJECT
    public:
        StackModel(QObject *parent = 0);

        void setAutoUpdate(bool autoUpdate);
        virtual void update()=0;
        FramesModel* modelForThread(int id);
        bool hasThreads() const;
    protected:
        bool m_autoUpdate;
};

}

#endif // KDEVELOP_STACKMODEL_H
