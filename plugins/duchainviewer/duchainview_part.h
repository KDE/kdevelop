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

#include <iplugin.h>

#include <QPointer>

class DUChainModel;
class QTreeView;

class DUChainViewPart : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    DUChainViewPart( QObject *parent, const QStringList & );
    virtual ~DUChainViewPart();

    // KDevelop::Plugin methods
    virtual void unload();

    DUChainModel* model() const;

private:
    DUChainModel* m_model;
    class DUChainViewFactory* m_factory;
};

#endif // DUCHAINVIEW_PART_H

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on
