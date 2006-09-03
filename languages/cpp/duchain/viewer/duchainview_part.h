/*
 * This file is part of KDevelop
 *
 * Copyright (c) 2006 Adam Treat <treat@kde.org>
 * Copyright (c) 2006 Hamish Rodda <rodda@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
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

#ifndef DUCHAINVIEW_PART_H
#define DUCHAINVIEW_PART_H

#include <kdevplugin.h>

class DUChainModel;
class QTreeView;

class DUChainViewPart : public KDevPlugin
{
    Q_OBJECT

public:
    DUChainViewPart( QObject *parent, const QStringList & );
    virtual ~DUChainViewPart();

    // KDevPlugin methods
    virtual QWidget *pluginView() const;
    virtual Qt::DockWidgetArea dockWidgetAreaHint() const;

private:
    DUChainModel* m_model;
    QTreeView* m_view;
};

#endif // DUCHAINVIEW_PART_H

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
