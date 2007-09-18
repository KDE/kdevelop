/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright (C) 2007 The KDevelop Authors <kdevelop-devel@kdevelop.org>                     *
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

#include "commitdlg.h"
#include <qevent.h>
#include <ktextedit.h>

CommitDlg::CommitDlg( QWidget* parent )
    : CommitDlgBase( parent )
{
    textMessage->installEventFilter(this);
}

bool CommitDlg::eventFilter( QObject* obj, QEvent* ev )
{
    if( ev->type() == QEvent::KeyPress )
    {
        QKeyEvent* k = static_cast<QKeyEvent*>(ev);
        if( ( k->key() == Qt::Key_Return || k->key() == Qt::Key_Enter ) && k->state() == Qt::ControlButton )
        {
            accept();
            return true;
        }
    }
    return false;
}

#include "commitdlg.moc"

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
