/* This file is part of KDevelop
Copyright 2008 Anreas Pakulat <apaku@gmx.de>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#ifndef SESSIONDIALOG_H
#define SESSIONDIALOG_H

#include <kdialog.h>
#include <QtCore/QHash>

namespace Ui
{
class SessionDialog;
}

namespace KDevelop
{
class Session;
}

class QListWidgetItem;

class SessionDialog : public KDialog
{
    Q_OBJECT
public:
    SessionDialog( QWidget* = 0 );
private Q_SLOTS:
    void createSession();
    void deleteSession();
    void renameSession( QListWidgetItem* );
    QListWidgetItem* createAndSetupItem( KDevelop::Session* );
private:
    void enableNewButton();
    Ui::SessionDialog* m_ui;
    QHash<QListWidgetItem*, KDevelop::Session*> itemSessionMap;
};

#endif