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

#include <debugger/util/treeitem.h>
#include "../debuggerexport.h"

namespace KDevelop
{
class StackModel;

class KDEVPLATFORMDEBUGGER_EXPORT StackItem : public TreeItem
{
    Q_OBJECT
    public:
        StackItem(StackModel* model, TreeItem* parent, const QString& prefix);
        int id() const;
        void setInformation(int id, const QString& name, const QPair<QString, int>& location);
        StackModel* stackModel();
        
        virtual void clicked();
        virtual QVariant icon(int column) const;
    private:
        QPair<QString, int> mLocation;
        QString mPrefix;
        StackModel* mModel;
        int mId;
};

}

#endif // STACKITEM_H
