/* This file is part of KDevelop
Copyright 2006 Adam Treat <treat@kde.org>
Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>
Copyright 2008 Andreas Pakuat <apaku@gmx.de>

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

#include "environmentwidget.h"

#include <QLabel>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMap>
#include <QProcess>

#include <kdialog.h>
#include <QLineEdit>
#include <ktextedit.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <kconfigdialogmanager.h>
#include <KLocalizedString>

#include "environmentgroupmodel.h"
#include "placeholderitemproxymodel.h"
#include "../debug.h"

namespace KDevelop
{

EnvironmentWidget::EnvironmentWidget( QWidget *parent )
        : QWidget( parent ), groupModel( new EnvironmentGroupModel() ), proxyModel( new QSortFilterProxyModel() )
{

    // setup ui
    ui.setupUi( this );
    ui.variableTable->verticalHeader()->hide();

    proxyModel->setSourceModel( groupModel );

    PlaceholderItemProxyModel* topProxyModel  = new PlaceholderItemProxyModel(this);
    topProxyModel->setSourceModel(proxyModel);
    topProxyModel->setColumnHint(0, i18n("Enter variable ..."));
    connect(topProxyModel, &PlaceholderItemProxyModel::dataInserted, this, &EnvironmentWidget::handleVariableInserted);

    ui.variableTable->setModel( topProxyModel );
    ui.variableTable->horizontalHeader()->setSectionResizeMode( 1, QHeaderView::Stretch );
    ui.addgrpBtn->setIcon(QIcon::fromTheme("list-add"));
    ui.removegrpBtn->setIcon(QIcon::fromTheme("list-remove"));
    ui.deleteButton->setIcon(QIcon::fromTheme("list-remove"));
    ui.deleteButton->setShortcut(Qt::Key_Delete);
    ui.newMultipleButton->setIcon(QIcon::fromTheme("format-list-unordered"));

    connect( ui.deleteButton, &QPushButton::clicked,
             this, &EnvironmentWidget::deleteButtonClicked );
    connect( ui.newMultipleButton, &QPushButton::clicked,
             this, &EnvironmentWidget::newMultipleButtonClicked );

    connect( ui.addgrpBtn, &QPushButton::clicked, this, &EnvironmentWidget::addGroupClicked );
    connect( ui.addgrpBtn, &QPushButton::clicked, this, &EnvironmentWidget::changed );
    connect( ui.removegrpBtn, &QPushButton::clicked, this, &EnvironmentWidget::removeGroupClicked );
    connect( ui.removegrpBtn, &QPushButton::clicked, this, &EnvironmentWidget::changed );
    connect( ui.setAsDefaultBtn, &QPushButton::clicked, this, &EnvironmentWidget::setAsDefault );
    connect( ui.setAsDefaultBtn, &QPushButton::clicked, this, &EnvironmentWidget::changed );
    connect( ui.activeCombo, static_cast<void(KComboBox::*)(int)>(&KComboBox::currentIndexChanged),
             this, &EnvironmentWidget::activeGroupChanged );
    connect( ui.activeCombo, &KComboBox::editTextChanged, this, &EnvironmentWidget::enableButtons);
    connect( groupModel, &EnvironmentGroupModel::dataChanged, this, &EnvironmentWidget::changed );
    connect( groupModel, &EnvironmentGroupModel::rowsRemoved, this, &EnvironmentWidget::changed );
    connect( groupModel, &EnvironmentGroupModel::rowsInserted, this, &EnvironmentWidget::changed );
    connect( groupModel, &EnvironmentGroupModel::rowsRemoved, this, &EnvironmentWidget::enableDeleteButton );
    connect( groupModel, &EnvironmentGroupModel::rowsInserted, this, &EnvironmentWidget::enableDeleteButton );
    connect( groupModel, &EnvironmentGroupModel::modelReset, this, &EnvironmentWidget::enableDeleteButton );
}

void EnvironmentWidget::setActiveGroup( const QString& group )
{
    ui.activeCombo->setCurrentItem(group);
}

void EnvironmentWidget::enableDeleteButton()
{
    ui.deleteButton->setEnabled( groupModel->rowCount() > 0 );
}

void EnvironmentWidget::setAsDefault()
{
    groupModel->changeDefaultGroup( ui.activeCombo->currentText() );
    enableButtons( ui.activeCombo->currentText() );
    emit changed();
}

void EnvironmentWidget::loadSettings( KConfig* config )
{
    qCDebug(SHELL) << "Loading groups from config";
    groupModel->loadFromConfig( config );

    ui.activeCombo->clear();

    QStringList groupList = groupModel->groups();
    qCDebug(SHELL) << "Grouplist:" << groupList << "default group:" << groupModel->defaultGroup();
    ui.activeCombo->addItems( groupList );
    int idx = ui.activeCombo->findText( groupModel->defaultGroup() );
    ui.activeCombo->setCurrentIndex( idx );
}

void EnvironmentWidget::saveSettings( KConfig* config )
{
    groupModel->saveToConfig( config );
}

void EnvironmentWidget::defaults( KConfig* config )
{
    loadSettings( config );
}

void EnvironmentWidget::deleteButtonClicked()
{
    QModelIndexList selected = ui.variableTable->selectionModel()->selectedRows();
    if( selected.isEmpty() )
        return;

    QStringList variables;
    foreach( const QModelIndex &idx, selected )
    {
        const QString variable = idx.data(EnvironmentGroupModel::VariableRole).toString();
        variables << variable;
    }

    groupModel->removeVariables(variables);
}

void EnvironmentWidget::handleVariableInserted(int /*column*/, const QVariant& value)
{
    groupModel->addVariable(value.toString(), QString());
}

void EnvironmentWidget::newMultipleButtonClicked()
{
    KDialog * dialog = new KDialog( this );
    dialog->setCaption( i18n( "New Environment Variables" ) );
    dialog->setButtons( KDialog::Ok | KDialog::Cancel );
    dialog->setDefaultButton( KDialog::Ok );

    QWidget *main = new QWidget( dialog );
    QVBoxLayout *layout = new QVBoxLayout( main );

    KTextEdit *edit = new KTextEdit( main );
    layout->addWidget( edit );
    edit->setPlaceholderText("VARIABLE1=VALUE1\nVARIABLE2=VALUE2");
    edit->setFocus();
    dialog->setMainWidget( main );

    if ( dialog->exec() != QDialog::Accepted ) {
        return;
    }

    QStringList lines = edit->toPlainText().split( "\n", QString::SkipEmptyParts );

    foreach(const QString &line, lines) {
        QString name = line.section('=', 0, 0);
        QString value = line.section('=', 1, -1).trimmed();
        if (!name.isEmpty() && !value.isEmpty()) {
            groupModel->addVariable( name, value );
        }
    }
}

void EnvironmentWidget::addGroupClicked()
{
    QString curText = ui.activeCombo->currentText();
    if( groupModel->groups().contains( curText ) )
    {
        return; // same group name cannot be added twice.
    }
    ui.activeCombo->addItem( curText );
    ui.activeCombo->setCurrentItem( curText );
}

void EnvironmentWidget::removeGroupClicked()
{
    int idx = ui.activeCombo->currentIndex();
    if( idx < 0 || ui.activeCombo->count() == 1 )
    {
        return;
    }

    QString curText = ui.activeCombo->currentText();
    groupModel->removeGroup( curText );
    ui.activeCombo->removeItem( idx );
    ui.activeCombo->setCurrentItem( groupModel->defaultGroup() );
}

void EnvironmentWidget::activeGroupChanged( int /*idx*/ )
{
    groupModel->setCurrentGroup( ui.activeCombo->currentText() );
    enableButtons( ui.activeCombo->currentText() );
}

void EnvironmentWidget::enableButtons( const QString& txt )
{
    ui.addgrpBtn->setEnabled( !groupModel->groups().contains( txt  ) );
    ui.removegrpBtn->setEnabled( ( groupModel->groups().contains( txt  ) && groupModel->defaultGroup() != txt ) );
    ui.setAsDefaultBtn->setEnabled( ( groupModel->groups().contains( txt  ) && groupModel->defaultGroup() != txt ) );
}


}

#include "moc_environmentwidget.cpp"
