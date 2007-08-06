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

#include "environmentwidget.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMap>
#include <QProcess>

#include <kdebug.h>
#include <kdialog.h>
#include <klineedit.h>
#include <ktextedit.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <kconfigdialogmanager.h>


namespace KDevelop
{

EnvironmentWidget::EnvironmentWidget( QWidget *parent )
        : QWidget( parent )
{

    // setup ui
    ui.setupUi( this );

//     ui.variableTable->setHorizontalHeaderLabels( QStringList()
//                 << i18nc( "Name of an environment variable", "Variable Name" )
//                 << i18nc( "Name of an environment variable", "Variable Name" ) );
    ui.variableTable->verticalHeader() ->hide();
    ui.variableTable->setSelectionMode( QAbstractItemView::NoSelection );
    ui.variableTable->horizontalHeader() ->setStretchLastSection ( true );
    connect( ui.variableTable, SIGNAL( cellChanged( int, int ) ),
             this, SLOT( settingsChanged( int, int ) ) );
    connect( ui.variableTable, SIGNAL( currentCellChanged( int, int, int, int ) ),
             this, SLOT( focusChanged( int, int, int, int ) ) );

    connect( ui.newButton, SIGNAL( clicked() ),
             this, SLOT( newButtonClicked() ) );
    connect( ui.deleteButton, SIGNAL( clicked() ),
             this, SLOT( deleteButtonClicked() ) );
    connect( ui.processDefaultButton, SIGNAL( clicked() ),
             this, SLOT( processDefaultButtonClicked() ) );

    connect( ui.addgrpBtn, SIGNAL( clicked() ), this, SLOT( addGroupClicked() ) );
    connect( ui.removegrpBtn, SIGNAL( clicked() ), this, SLOT( removeGroupClicked() ) );
    connect( ui.activeCombo, SIGNAL(currentIndexChanged(int)),
             this, SLOT( activeGroupChanged(int)) );

    ui.deleteButton->setEnabled( false );
    ui.processDefaultButton->setEnabled( false );

    // setup process default environment map.
    QStringList procDefaultList = QProcess::systemEnvironment();
    foreach( QString _line, procDefaultList )
    {
        QString varName = _line.section( '=', 0, 0 );
        QString varValue = _line.section( '=', 1 );
        m_procDefaultMaps.insert( varName, varValue );
    }
}

void EnvironmentWidget::loadSettings( KConfig* config )
{
    kDebug(9508) << "Loading groups from config";
    m_groups.loadSettings( config );

    ui.activeCombo->clear();
    ui.variableTable->clearContents();

    ui.activeCombo->blockSignals( true );
    QList<QString> groupList = m_groups.groups();
    ui.activeCombo->addItems( m_groups.groups() );
    if( !groupList.contains( m_groups.defaultGroup() ) )
    {
        ui.activeCombo->addItem( m_groups.defaultGroup() );
    }
    int idx = ui.activeCombo->findText( m_groups.defaultGroup() );
    ui.activeCombo->blockSignals( false );
    ui.activeCombo->setCurrentIndex( idx );
}

void EnvironmentWidget::saveSettings( KConfig* config )
{
    m_groups.saveSettings( config );
}

void EnvironmentWidget::defaults( KConfig* config )
{
    loadSettings( config );
}

void EnvironmentWidget::newButtonClicked()
{
    KDialog * dialog = new KDialog( this );
    dialog->setCaption( i18n( "New Environment Variable" ) );
    dialog->setButtons( KDialog::Ok | KDialog::Cancel );
    dialog->setDefaultButton( KDialog::Ok );

    QWidget *main = new QWidget( dialog );
    QGridLayout *layout = new QGridLayout( main );

    QLabel* l = new QLabel( i18nc( "Name of an environment variable", "Variable Name:" ), main );
    l->setAlignment( Qt::AlignRight | Qt::AlignTop );
    layout->addWidget( l, 0, 0 );
    KLineEdit *nameEdit = new KLineEdit( main );
    layout->addWidget( nameEdit, 0, 1 );
    l = new QLabel( i18nc( "Name of an environment variable", "Variable Value:" ), main );
    l->setAlignment( Qt::AlignRight | Qt::AlignTop );
    layout->addWidget( l, 1, 0 );
    KTextEdit *valueEdit = new KTextEdit( main );
    layout->addWidget( valueEdit, 1, 1 );
    nameEdit->setFocus();
    dialog->setMainWidget( main );

    if ( dialog->exec() == QDialog::Accepted )
    {
        QString _name = nameEdit->text();
        QString _value = valueEdit->toPlainText();
        if ( _name.isEmpty() )
            return ; //message box?

//         generateCurrentMaps();
//         if ( m_currentOverrides.contains( _name )
//              || m_currentProcessDefaults.contains( _name ) )
//             return ; //message box?


        // If no default group, create group named with 'default'
        if( ui.activeCombo->count() < 1 )
        {
            ui.activeCombo->addItem( "default" );
            m_groups.variables( "default" );
            m_groups.setDefaultGroup( "default" );
        }

        // Add it to QMap
        QString currentGroup = ui.activeCombo->currentText();
        m_groups.variables( currentGroup ).insert( _name, _value );

        ui.variableTable->blockSignals( true );

        // Add it at the top?
        int row = ui.variableTable->rowCount();
        ui.variableTable->insertRow( row );

        QTableWidgetItem * name = new QTableWidgetItem( _name );
        setOverride( name );
        ui.variableTable->setItem( row, 0, name );
        name->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );

        QTableWidgetItem * value = new QTableWidgetItem( _value );
        setOverride( value );
        ui.variableTable->setItem( row, 1, value );

        //Make sure it is visible
        ui.variableTable->scrollToItem( name );

        ui.variableTable->blockSignals( false );

        ui.variableTable->setCurrentItem( name );

        ui.deleteButton->setEnabled( true );
        if( diff() )
            emit changed();
    }
}

void EnvironmentWidget::deleteButtonClicked()
{
    QString curGroup = ui.activeCombo->currentText();
    if( curGroup.isEmpty() )
    {
        return;
    }

    int row = ui.variableTable->currentRow();
    if( m_groups.groups().contains( curGroup ) )
    {

        QTableWidgetItem *item = ui.variableTable->item( row, 0 );
        if( !item ) return;
        m_groups.variables( curGroup ).remove( item->text() );
    }

    ui.variableTable->removeRow( row );
    ui.processDefaultButton->setEnabled( false );
    if( ui.variableTable->rowCount() < 1 )
        ui.deleteButton->setEnabled( false );
    if( diff() )
        emit changed();
}

void EnvironmentWidget::processDefaultButtonClicked()
{
    //@TODO setup process default in other ways
    int row = ui.variableTable->currentRow();
    QTableWidgetItem * name = ui.variableTable->item( row, 0 );
    QTableWidgetItem * value = ui.variableTable->item( row, 1 );
    if( name==0 || value==0 )
        return;
    QString _name = name->text();

    if( !m_procDefaultMaps.contains( _name ) )
        return;

//     QString pValue = Core::environment() ->processDefault( _name );
    QString pValue = m_procDefaultMaps[_name];

    value->setText( pValue );
    setProcessDefault( name );
    setProcessDefault( value );
    ui.processDefaultButton->setEnabled( false );
}

void EnvironmentWidget::settingsChanged( int row, int /*column*/ )
{
    QTableWidgetItem * name = ui.variableTable->item( row, 0 );
    QTableWidgetItem * value = ui.variableTable->item( row, 1 );
    QString _name = name->text();
    QString _value = value->text();
    QString currentGroup = ui.activeCombo->currentText();

    m_groups.variables( currentGroup )[ _name ] = _value;

//     bool o = isOverride( name );
//     bool p = isProcessDefault( name );
//     QString oValue = Core::environment() ->override( _name );
//     QString pValue = Core::environment() ->processDefault( _name );
//
//     // 1. Process default is changed to override
//     if ( p && _value != pValue )
//     {
//         setOverride( name );
//         setOverride( value );
//     }
//     // 2. Override is changed back to process default
//     else if ( o && _value == pValue )
//     {
//         setProcessDefault( name );
//         setProcessDefault( value );
//     }
    emit changed();
}

void EnvironmentWidget::focusChanged( int row, int, int, int )
{
    if( row < 0 ) return;

    QTableWidgetItem *nameItem = ui.variableTable->item( row, 0 );
    QTableWidgetItem *valueItem = ui.variableTable->item( row, 1 );
    if( nameItem && valueItem )
    {
        QString name = nameItem->text();
        QString value = valueItem->text();

        // Var is processDefault and value is changed?
        bool pDefault = false;
        QString pValue;
        if( m_procDefaultMaps.contains(name) )
        {
            pDefault = true;
            QString pValue = m_procDefaultMaps[name];
        }
        ui.processDefaultButton->setEnabled( pDefault && pValue != value );

        // why do we disable delete button here?
    //     // Var is not processDefault?
    //     ui.deleteButton->setEnabled( !pDefault );
    }
}

void EnvironmentWidget::addGroupClicked()
{
    QString curText = ui.activeCombo->currentText();
    if( m_groups.groups().contains( curText ) )
    {
        return; // same group name cannot be added twice.
    }
//     m_enabled = curText;
//     ui.variableTable->clear();
    ui.activeCombo->addItem( curText ); // TODO test
    int idx = ui.activeCombo->findText( curText );
    activeGroupChanged( idx );
}

void EnvironmentWidget::removeGroupClicked()
{
    int idx = ui.activeCombo->currentIndex();
    if( idx < 0 || ui.activeCombo->count() == 1 )
    {
        return;
    }

    QString curText = ui.activeCombo->currentText();
    m_groups.removeGroup( curText );
//     ui.variableTable->clear();
    ui.activeCombo->removeItem( idx );
    // Should call activeGroupChanged explicitly?
    // No. called by signal-slot automatically.
    activeGroupChanged( idx );
}

void EnvironmentWidget::activeGroupChanged( int idx )
{
    ui.variableTable->clearContents();

    m_groups.setDefaultGroup( ui.activeCombo->currentText() );
    ui.variableTable->blockSignals(true);

    QString group = m_groups.defaultGroup();

    foreach( QString var, m_groups.variables( group ) )
    {
        //Add it at the top?
        int row = ui.variableTable->rowCount();
        ui.variableTable->insertRow( row );

        QTableWidgetItem * name = new QTableWidgetItem( var );
        setOverride( name );
        ui.variableTable->setItem( row, 0, name );
        name->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );

        QTableWidgetItem * value = new QTableWidgetItem( m_groups.variables( group )[var]  );
        setOverride( value );
        ui.variableTable->setItem( row, 1, value );

        //Make sure it is visible
        ui.variableTable->scrollToItem( name );
    }
    ui.variableTable->blockSignals(false);

    if( ui.variableTable->rowCount() > 0 )
        ui.deleteButton->setEnabled( true );
    else
        ui.deleteButton->setEnabled( false );

    emit changed();
}

bool EnvironmentWidget::isOverride( QTableWidgetItem *item ) const
{
    return ( item->textColor() == Qt::red );
}

bool EnvironmentWidget::isProcessDefault( QTableWidgetItem *item ) const
{
    return ( item->textColor() == Qt::black );
}

void EnvironmentWidget::setOverride( QTableWidgetItem *item )
{
    item->setTextColor( Qt::red );
}

void EnvironmentWidget::setProcessDefault( QTableWidgetItem * item )
{
    item->setTextColor( Qt::black );
}

void EnvironmentWidget::generateCurrentMaps()
{
    // TODO FIX
    //create maps of current settings to compare
//     m_currentOverrides.clear();
//     m_currentProcessDefaults.clear();
//     int rows = ui.variableTable->rowCount();
//     for ( int row = 0; row < rows; ++row )
//     {
//         QTableWidgetItem * name = ui.variableTable->item( row, 0 );
//         QTableWidgetItem * value = ui.variableTable->item( row, 1 );
//         QString _name = name->text();
//         QString _value = value->text();
//         if ( isOverride( name ) )
//             m_currentOverrides.insert( _name, _value );
//         else if ( isProcessDefault( name ) )
//             m_currentProcessDefaults.insert( _name, _value );
//     }
}

bool EnvironmentWidget::diff()
{
    // TODO FIX
    return true;
//     generateCurrentMaps();
//     return ( m_currentOverrides != m_overrides
//              || m_currentProcessDefaults != m_processDefaults );
}

}

#include "environmentwidget.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
