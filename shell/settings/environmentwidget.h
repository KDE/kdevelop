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

#ifndef ENVIRONMENTWIDGET_H
#define ENVIRONMENTWIDGET_H

#include <QtGui/QWidget>
#include "ui_environmentwidget.h"
#include <environmentgrouplist.h>

class KConfig;
class QTableWidgetItem;


namespace KDevelop
{

/**
 * @short Environment variable setting widget.
 * This class manages a EnvironmentGroupList and allows to change the variables and add/remove groups
 *
 * @sa EnvPreferences
 */
class EnvironmentWidget: public QWidget
{
    Q_OBJECT
//     Q_PROPERTY( QStringList items READ items WRITE setItems USER true )

public:
    explicit EnvironmentWidget( QWidget *parent = 0 );

    void loadSettings( KConfig* config );
    void saveSettings( KConfig* config );
    void defaults( KConfig* config );

Q_SIGNALS:
    void changed();

private Q_SLOTS:
    void newButtonClicked();
    void deleteButtonClicked();
    void processDefaultButtonClicked();
    void settingsChanged( int row, int column );
    void focusChanged( int row, int, int, int );
    void addGroupClicked();
    void removeGroupClicked();
    void activeGroupChanged(int);

private:
//     void load( bool defaults );
    bool isOverride( QTableWidgetItem *item ) const;
    bool isProcessDefault( QTableWidgetItem *item ) const;
    void setOverride( QTableWidgetItem *item );
    void setProcessDefault( QTableWidgetItem *item );
    void generateCurrentMaps();
    bool diff();

private:
    Ui::EnvironmentWidget ui;
    QString m_enabled;
    QMap< QString, QString > m_procDefaultMaps;
    EnvironmentGroupList m_groups;
    
//     QMap<QString, QString> m_overrides;
//     QMap<QString, QString> m_processDefaults;
//     QMap<QString, QString> m_currentOverrides;
//     QMap<QString, QString> m_currentProcessDefaults;
};

}

#endif

