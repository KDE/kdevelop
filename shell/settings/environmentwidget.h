/* This file is part of KDevelop
Copyright 2006 Adam Treat <treat@kde.org>
Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>

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

#ifndef KDEVPLATFORM_ENVIRONMENTWIDGET_H
#define KDEVPLATFORM_ENVIRONMENTWIDGET_H

#include <QWidget>
#include "ui_environmentwidget.h"

class KConfig;
class QSortFilterProxyModel;


namespace KDevelop
{

class EnvironmentGroupModel;


/**
 * @short Environment variable setting widget.
 * This class manages a EnvironmentGroupList and allows to change the variables and add/remove groups
 *
 * @sa EnvPreferences
 */
class EnvironmentWidget: public QWidget
{
    Q_OBJECT

public:
    explicit EnvironmentWidget( QWidget *parent = 0 );

    void loadSettings( KConfig* config );
    void saveSettings( KConfig* config );
    void defaults( KConfig* config );
    void setActiveGroup( const QString& group );

Q_SIGNALS:
    void changed();

private Q_SLOTS:
    void handleVariableInserted(int column, const QVariant& value);
    void deleteButtonClicked();
    void newMultipleButtonClicked();
    void addGroupClicked();
    void removeGroupClicked();
    void activeGroupChanged( int );
    void enableDeleteButton();
    void setAsDefault();
    void enableButtons( const QString& );

private:
    Ui::EnvironmentWidget ui;
    EnvironmentGroupModel* groupModel;
    QSortFilterProxyModel* proxyModel;

};

}

#endif

