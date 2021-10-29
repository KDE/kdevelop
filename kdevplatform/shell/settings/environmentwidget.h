/*
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
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

