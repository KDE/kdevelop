/***************************************************************************
 *   This file is part of KDevelop                                         *
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

#ifndef KDEVPLATFORM_VCSIMPORTMETADATAWIDGET_H
#define KDEVPLATFORM_VCSIMPORTMETADATAWIDGET_H

#include <QWidget>
#include "../vcsexport.h"
#include <vcs/interfaces/ibasicversioncontrol.h>

class KUrl;

namespace KDevelop
{

class VcsLocation;

class KDEVPLATFORMVCS_EXPORT VcsImportMetadataWidget : public QWidget
{
Q_OBJECT
public:
    VcsImportMetadataWidget( QWidget* parent );
    virtual ~VcsImportMetadataWidget();
    virtual KUrl source() const = 0;
    virtual VcsLocation destination() const = 0;
    virtual QString message() const = 0;

    /**
     * Check whether the given data is valid.
     * @returns true if all data in the widget is valid
     */
    virtual bool hasValidData() const = 0;
    
    /**
     * Select whether the widget should re-use the last part of the source location
     * for the destination. The default implementation simply ignores this setting.
     */
    virtual void setUseSourceDirForDestination( bool ) {}
Q_SIGNALS:
    void changed();
public Q_SLOTS:
    virtual void setSourceLocation( const VcsLocation& ) = 0;
    virtual void setSourceLocationEditable( bool ) = 0;
};

}

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
