/***************************************************************************
*   Copyright (C) 2006 by Andreas Pakulat                                 *
*   apaku@gmx.de                                                          *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "disablesubprojectdlg.h"
#include <klistview.h>
#include <qstringlist.h>
#include "scope.h"
#include "qmakescopeitem.h"

DisableSubprojectDlg::DisableSubprojectDlg( const QStringList& projects, QWidget* parent, const char* name, WFlags fl )
        : DisableSubprojectDlgBase( parent, name, fl )
{
    for( QStringList::const_iterator it = projects.begin(); it != projects.end(); ++it )
    {
        new QCheckListItem(subprojects_view, *it, QCheckListItem::CheckBox);
    }
}

DisableSubprojectDlg::~DisableSubprojectDlg()
{
}

QStringList DisableSubprojectDlg::selectedProjects()
{
    QStringList result;
    QListViewItem* item = subprojects_view->firstChild();
    while( item )
    {
        QCheckListItem* ci = dynamic_cast<QCheckListItem*>(item);
        if( ci && ci->isOn() )
        {
            result << ci->text(0);
        }
        item = item->nextSibling();
    }
    return result;
}

#include "disablesubprojectdlg.moc"


// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
