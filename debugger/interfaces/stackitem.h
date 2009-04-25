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

#ifndef STACKITEM_H
#define STACKITEM_H

#include <QtCore/QPair>

#include <debugger/util/treeitem.h>
#include "../debuggerexport.h"

namespace KDevelop
{
class StackModel;
class FramesModel;

class KDEVPLATFORMDEBUGGER_EXPORT ThreadItem : public TreeItem
{
    Q_OBJECT
    public:
        ThreadItem(StackModel* model);
        int id() const;
        StackModel* stackModel();
        FramesModel* framesModel();
        
        void setInformation(int id, const QString& name);
    private:
        int mId;
        FramesModel* mFramesModel;
        StackModel* mStackModel;
};

class KDEVPLATFORMDEBUGGER_EXPORT FrameItem : public TreeItem
{
    Q_OBJECT
    public:
        FrameItem(FramesModel* model);
        int id() const;
        void setInformation(int id, const QString& name, const QString& file, int line);
        FramesModel* framesModel();
        
        virtual void clicked();
        virtual QVariant icon(int column) const;
        
    private:
        virtual void fetchMoreChildren() {}
        
        QPair<QString, int> mLocation;
        FramesModel* mModel;
        int mId;
};

}

#endif // STACKITEM_H
