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

namespace KDevelop {


class KDEVPLATFORMDEBUGGER_EXPORT StackModel : public TreeModel
{
    Q_OBJECT
public:
    StackModel(const QVector<QString>& headers, QObject *parent = 0);

    void setAutoUpdate(bool autoUpdate);
    virtual void update()=0;

protected:
    bool m_autoUpdate;
};

}

#endif // KDEVELOP_STACKMODEL_H
