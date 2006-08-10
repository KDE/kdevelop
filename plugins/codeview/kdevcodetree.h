/*
 * This file is part of KDevelop
 *
 * Copyright (c) 2006 Adam Treat <treat@kde.org>
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

#ifndef KDEVCODETREE_H
#define KDEVCODETREE_H

#include "kdevtreeview.h"

class KUrl;
class KDevDocument;
class KDevCodeProxy;

class KDevCodeTree: public KDevTreeView
{
    Q_OBJECT
public:
    KDevCodeTree( QWidget *parent );
    virtual ~KDevCodeTree();

    KDevCodeProxy *codeProxy() const;

private slots:
    void documentActivated( KDevDocument* file );
    void activated( const QModelIndex &index );
    void modeCurrent();
    void modeNormalize();
    void modeAggregate();
    void filterKind();
    void popupContextMenu( const QPoint &pos );

private:
    bool m_trackCurrent;
    int m_kindFilter;
};

#endif // KDEVCODETREE_H

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
