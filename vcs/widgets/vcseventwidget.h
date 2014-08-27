/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_VCSEVENTWIDGET_H
#define KDEVPLATFORM_VCSEVENTWIDGET_H

#include <QWidget>
#include "vcsexport.h"

class QPoint;
class QModelIndex;
class KUrl;

namespace KDevelop
{
class VcsRevision;
class IBasicVersionControl;


class KDEVPLATFORMVCS_EXPORT VcsEventWidget : public QWidget
{
    Q_OBJECT
public:
    VcsEventWidget( const KUrl& url, const VcsRevision& rev, KDevelop::IBasicVersionControl* iface, QWidget* parent = 0 );
    virtual ~VcsEventWidget();

private:
    Q_PRIVATE_SLOT(d, void diffToPrevious())
    Q_PRIVATE_SLOT(d, void diffRevisions())
    Q_PRIVATE_SLOT(d, void eventViewCustomContextMenuRequested( const QPoint &point ))
    Q_PRIVATE_SLOT(d, void eventViewClicked( const QModelIndex &index ))
    Q_PRIVATE_SLOT(d, void currentRowChanged(const QModelIndex&, const QModelIndex& ))
    Q_PRIVATE_SLOT(d, void copyRevision())
    class VcsEventWidgetPrivate* const d;
};
}

#endif
