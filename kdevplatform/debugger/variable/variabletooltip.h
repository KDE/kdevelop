/*
    SPDX-FileCopyrightText: 2008 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_VARIABLETOOLTIP_H
#define KDEVPLATFORM_VARIABLETOOLTIP_H

#include "../../util/activetooltip.h"

class QItemSelectionModel;
class QString;

namespace KDevelop
{
    class Variable;
    class TreeModel;
    class TreeItem;
    class AsyncTreeView;

    class VariableToolTip : public ActiveToolTip
    {
    Q_OBJECT
    public:
        VariableToolTip(QWidget* parent, const QPoint& position,
                        const QString& identifier);
        Variable* variable() const { return m_var; };
    private Q_SLOTS:
        void variableCreated(bool hasValue);
        void slotLinkActivated(const QString& link);
        void slotRangeChanged(int min, int max);

    private:
        TreeModel* m_model;
        Variable* m_var;
        QItemSelectionModel* m_selection;
        int m_itemHeight;
        AsyncTreeView* m_view;
    };
}

#endif
