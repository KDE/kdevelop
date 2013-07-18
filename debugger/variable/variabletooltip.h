/*
 * KDevelop Debugger Support
 *
 * Copyright 2008 Vladimir Prus <ghost@cs.msu.su>
 * Copyright 2009 Niko Sams <niko.sams@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_VARIABLETOOLTIP_H
#define KDEVPLATFORM_VARIABLETOOLTIP_H

#include "../../util/activetooltip.h"
#include "../util/treeview.h"

#include <QPoint>

class QItemSelectionModel;
class QString;
class QResizeEvent;

namespace KDevelop
{
    class Variable;
    class TreeModel;
    class TreeItem;

    class VariableToolTip : public ActiveToolTip
    {
        Q_OBJECT
        public:
        VariableToolTip(QWidget* parent, QPoint position, 
                        const QString& identifier);
    private slots:
        void variableCreated(bool hasValue);
        void slotLinkActivated(const QString& link);
        void slotRangeChanged(int min, int max);

    private:
        TreeModel* model_;
        Variable* var_;
        QItemSelectionModel* selection_;
        int itemHeight_;
        AsyncTreeView* view_;
    };
}

#endif
