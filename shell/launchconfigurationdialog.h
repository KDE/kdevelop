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

#ifndef KDEVPLATFORM_LAUNCHCONFIGURATIONDIALOG_H
#define KDEVPLATFORM_LAUNCHCONFIGURATIONDIALOG_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtGui/QItemEditorCreatorBase>

#include <kdialog.h>
#include <kcombobox.h>
#include <qitemeditorfactory.h>

#include "ui_launchconfigurationdialog.h"
#include <QStyledItemDelegate>

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
class ILauncher;
class LaunchConfigurationPageFactory;
class ILaunchMode;

class LaunchConfigurationType;
class LaunchConfiguration;
class LaunchConfigurationPage;
class ILaunchConfiguration;
class IProject;

class LaunchConfigurationModelDelegate : public QStyledItemDelegate
{
public:
    LaunchConfigurationModelDelegate();
    virtual QWidget* createEditor ( QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index ) const;
    virtual void setEditorData ( QWidget* editor, const QModelIndex& index ) const;
    virtual void setModelData ( QWidget* editor, QAbstractItemModel* model, const QModelIndex& index ) const;
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
    LaunchConfiguration* configForIndex( const QModelIndex& ) const;
    ILaunchMode* modeForIndex( const QModelIndex& idx ) const;
    QModelIndex indexForConfig( LaunchConfiguration* ) const;
    void addConfiguration(KDevelop::ILaunchConfiguration* launch, const QModelIndex& idx);
    KDevelop::IProject* projectForIndex(const QModelIndex& idx);
    
private:
    class TreeItem
    {
    public:
        TreeItem() : parent(0) {}
        virtual ~TreeItem() {}
        TreeItem* parent;
        int row;
        QList<TreeItem*> children;
    };
    class ProjectItem : public TreeItem
    {
    public:
        IProject* project;
    };
    class LaunchItem : public TreeItem
    {
    public:
        LaunchConfiguration* launch;
    };
    class LaunchModeItem : public TreeItem
    {
    public:
        ILaunchMode* mode;
    };
    class GenericPageItem : public TreeItem
    {
    public:
        QString text;
    };
    void addItemForLaunchConfig( LaunchConfiguration* l );
    void addLaunchModeItemsForLaunchConfig ( KDevelop::LaunchConfigurationsModel::LaunchItem* l );
    QList<TreeItem*> topItems;
    
public:
    ProjectItem* findItemForProject( IProject* );
};

class LaunchConfigPagesContainer : public QWidget
{
Q_OBJECT
public:
    LaunchConfigPagesContainer( const QList<LaunchConfigurationPageFactory*> &, QWidget* parent = 0 );
    void setLaunchConfiguration( LaunchConfiguration* );
    void save();
signals:
    void changed();
private:
    LaunchConfiguration* config;
    QList<LaunchConfigurationPage*> pages;
};

class LaunchConfigurationDialog : public KDialog, public Ui::LaunchConfigurationDialog
{
Q_OBJECT
public:
    LaunchConfigurationDialog(QWidget* parent = 0 );
    virtual QSize sizeHint() const;

private slots:
    void deleteConfiguration();
    void createConfiguration();
    void addConfiguration(KDevelop::ILaunchConfiguration*);
    void selectionChanged(QItemSelection,QItemSelection);
    void modelChanged(QModelIndex,QModelIndex);
    void pageChanged();
    void saveConfig();
    void updateNameLabel( LaunchConfiguration* l );
    void createEmptyLauncher();
    void launchModeChanged(int index);

private:
    void saveConfig( const QModelIndex& );
    LaunchConfigurationsModel* model;
    QMap<LaunchConfigurationType*, LaunchConfigPagesContainer*> typeWidgets;
    QMap<ILauncher*, LaunchConfigPagesContainer*> launcherWidgets;
    bool currentPageChanged;
private slots:
    void doTreeContextMenu(QPoint point);
    void renameSelected();
};

}

#endif

