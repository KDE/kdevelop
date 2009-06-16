/*  This file is part of KDevelop
    Copyright 2009 Andreas Pakulat <apaku@gmx.de>

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

#ifndef LAUNCHCONFIGURATIONDIALOG_H
#define LAUNCHCONFIGURATIONDIALOG_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtGui/QItemEditorCreatorBase>

#include <kdialog.h>
#include <kcombobox.h>
#include <qitemeditorfactory.h>

#include "ui_launchconfigurationdialog.h"

class QTreeView;
class QStackedWidget;
class QToolButton;
class QItemSelection;
class KTabWidget;

namespace Ui
{
class LaunchConfigTypePage;
}

namespace KDevelop
{

class LaunchConfigurationType;
class LaunchConfiguration;
class LaunchConfigurationPage;
class ILaunchConfiguration;
class IProject;

class LaunchConfigurationTypeComboBox : public KComboBox
{
Q_OBJECT
    Q_PROPERTY( QVariant currentData READ currentData WRITE setCurrentData NOTIFY currentDataChanged )
public:
    LaunchConfigurationTypeComboBox( QWidget* parent );
    QVariant currentData() const;
    void setCurrentData( const QVariant& );
signals:
    void currentDataChanged( const QVariant& );
private slots:
    void changeCurrentIndex(int);
};

class LaunchConfigurationTypeEditorCreator : public QItemEditorCreatorBase
{
public:
    LaunchConfigurationTypeEditorCreator();
    virtual QWidget* createWidget(QWidget* parent) const;
    virtual QByteArray valuePropertyName() const;
};


class LaunchConfigurationsModel : public QAbstractItemModel
{
Q_OBJECT
public:
    LaunchConfigurationsModel(QObject* parent = 0);
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& child) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
    void createConfiguration( const QModelIndex& );
    void deleteConfiguration( const QModelIndex& index );
    LaunchConfiguration* configForIndex( const QModelIndex& );
    QModelIndex indexForConfig( LaunchConfiguration* );
private:
    class TreeItem
    {
    public:
        TreeItem() : project(0), launch(0), parent(0) {}
        IProject* project;
        LaunchConfiguration* launch;
        TreeItem* parent;
        int row;
        QList<TreeItem*> children;
    };
    TreeItem* findItemForProject( IProject* );
    QList<TreeItem*> topItems;
};

class LaunchConfigTypePage : public QWidget
{
Q_OBJECT
public:
    LaunchConfigTypePage( LaunchConfigurationType*, QWidget* parent = 0 );
    void setLaunchConfiguration( LaunchConfiguration* );
    void save();
signals:
    void changed();
private slots:
    void changeMode(int);
    void changeLauncher(int);
private:
    Ui::LaunchConfigTypePage* ui;
    QMap<QString,QStringList> launchersForModes;
    LaunchConfiguration* config;
    QList<LaunchConfigurationPage*> launcherPages;
};

class LaunchConfigurationDialog : public KDialog, public Ui::LaunchConfigurationDialog
{
Q_OBJECT
public:
    LaunchConfigurationDialog(QWidget* parent = 0 );

private slots:
    void deleteConfiguration();
    void createConfiguration();
    void selectedConfig(QItemSelection,QItemSelection);
    void pageChanged();
    void saveConfig();
    void updateNameLabel( const QString& );
private:
    void saveConfig( const QModelIndex& );
    LaunchConfigurationsModel* model;
    QMap<LaunchConfigurationType*, LaunchConfigTypePage*> typeWidgets;
    bool currentPageChanged;
};

}

#endif

