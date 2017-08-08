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

class EnvironmentProfileListModel;
class EnvironmentProfileModel;


/**
 * @short Environment variable setting widget.
 * This class manages a EnvironmentProfileList and allows one to change the variables and add/remove groups
 *
 * @sa EnvPreferences
 */
class EnvironmentWidget: public QWidget
{
    Q_OBJECT

public:
    explicit EnvironmentWidget( QWidget *parent = nullptr );

    void loadSettings( KConfig* config );
    void saveSettings( KConfig* config );
    void defaults( KConfig* config );
    void selectProfile(const QString& profileName);

Q_SIGNALS:
    void changed();

private:
    QString askNewProfileName(const QString& defaultName);
    void removeSelectedVariables();
    void batchModeEditButtonClicked();
    void addProfile();
    void cloneSelectedProfile();
    void removeSelectedProfile();
    void setSelectedProfileAsDefault();
    void onDefaultProfileChanged(int defaultProfileIndex);
    void onSelectedProfileChanged(int selectedProfileIndex);
    void onVariableInserted(int column, const QVariant& value);
    void updateDeleteVariableButton();

private:
    Ui::EnvironmentWidget ui;
    EnvironmentProfileListModel* const m_environmentProfileListModel;
    EnvironmentProfileModel* const m_environmentProfileModel;
    QSortFilterProxyModel* const m_proxyModel;

};

}

#endif

