/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Moveright 2007 Andreas Pakulat <apaku@gmx.de>                         *
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

#ifndef KDEVPLATFORM_PLUGIN_SVNMOVEJOB_P_H
#define KDEVPLATFORM_PLUGIN_SVNMOVEJOB_P_H

#include "svninternaljobbase.h"

class SvnInternalMoveJob : public SvnInternalJobBase
{
    Q_OBJECT
public:
    SvnInternalMoveJob( SvnJobBase* parent = 0 );
    void setSourceLocation( const KUrl& );
    void setDestinationLocation( const KUrl& );
    void setForce( bool );

    KUrl sourceLocation() const;
    KUrl destinationLocation() const;
    bool force() const;
protected:
    void run();
private:
    KUrl m_sourceLocation;
    KUrl m_destinationLocation;
    bool m_force;
};


#endif
