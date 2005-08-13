 /**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "connectiontable.h"

#include <klocale.h>

ConnectionTable::ConnectionTable( QWidget *parent, const char *name )
    : QTable( 0, 4, parent, name )
{
    setSorting( TRUE );
    setShowGrid( FALSE );
    setFocusStyle( FollowStyle );
    setSelectionMode( SingleRow );
    horizontalHeader()->setLabel( 0, i18n( "Sender" ) );
    horizontalHeader()->setLabel( 1, i18n( "Signal" ) );
    horizontalHeader()->setLabel( 2, i18n( "Receiver" ) );
    horizontalHeader()->setLabel( 3, i18n( "Slot" ) );
    setColumnStretchable( 0, TRUE );
    setColumnStretchable( 1, TRUE );
    setColumnStretchable( 2, TRUE );
    setColumnStretchable( 3, TRUE );
}

void ConnectionTable::sortColumn( int col, bool ascending, bool )
{
    horizontalHeader()->setSortIndicator( col, ascending );
    if ( isEditing() )
	endEdit( currEditRow(), currEditCol(), FALSE, FALSE );
    QTable::sortColumn( col, ascending, TRUE );
    setCurrentCell( 0, 0 );
    emit resorted();
}
