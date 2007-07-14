/* This file is part of KDevelop
Copyright (C) 2006 Adam Treat <treat@kde.org>
Copyright (C) 2007 Dukju Ahn <dukjuahn@gmail.com>

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

#include "envwidget.h"
#include "ui_envsettings.h"

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

class EnvWidget::Private
{
public:
    Ui::EnvironmentWidget ui;
//     QMap<QString, QString> m_overrides;
//     QMap<QString, QString> m_processDefaults;
//     QMap<QString, QString> m_currentOverrides;
//     QMap<QString, QString> m_currentProcessDefaults;
    QString m_enabled;
    QMap< QString, QMap<QString,QString> > m_maps;
    QMap< QString, QString > m_procDefaultMaps;

    KConfig *m_config;
    QString m_confgroup;
//     QString m_entry;

    static QMap<QString, QString> environmentPairForPvt( KConfigGroup &cfg, const QString &profile )
    {
        QMap< QString, QString > ret;
        QStringList pairs = cfg.readEntry( "Environment Variables", QStringList() );

        foreach( QString _oneItem, pairs )
        {
            QString leftValue = _oneItem.section( '=', 0, 0 );
            QString varValue  = _oneItem.section( '=', 1 );

            QString group = leftValue.section( '_', 0, 0 );
            QString varName = leftValue.section( '_', 1 );

            if( group == profile )
            {
                kDebug() << k_funcinfo << " Adding to map " << varName << " :: " << varValue << endl;
                ret[varName] = varValue;
            }
        }

        return ret;
    }

};

EnvWidget::EnvWidget( QWidget *parent )
        : QWidget( parent ), d(new EnvWidget::Private)
{
//     // KConfigXT stuffs
//     QHash<QString, QByteArray> *changeHash = KConfigDialogManager::changedMap();
//     if( !changeHash->contains( "EnvWidget" ) )
//         changeHash->insert( "EnvWidget", SIGNAL(changed()) );

    // setup ui
    d->ui.setupUi( this );
    d->ui.variableTable->setColumnCount( 2 );
    d->ui.variableTable->setRowCount( 0 );
    d->ui.variableTable->verticalHeader() ->hide();
    d->ui.variableTable->setSelectionMode( QAbstractItemView::NoSelection );
    d->ui.variableTable->horizontalHeader() ->setStretchLastSection ( true );
    connect( d->ui.variableTable, SIGNAL( cellChanged( int, int ) ),
             this, SLOT( settingsChanged( int, int ) ) );
    connect( d->ui.variableTable, SIGNAL( currentCellChanged( int, int, int, int ) ),
             this, SLOT( focusChanged( int, int, int, int ) ) );

    connect( d->ui.newButton, SIGNAL( clicked() ),
             this, SLOT( newButtonClicked() ) );
    connect( d->ui.deleteButton, SIGNAL( clicked() ),
             this, SLOT( deleteButtonClicked() ) );
    connect( d->ui.processDefaultButton, SIGNAL( clicked() ),
             this, SLOT( processDefaultButtonClicked() ) );

    connect( d->ui.addgrpBtn, SIGNAL( clicked() ), this, SLOT( addGroupClicked() ) );
    connect( d->ui.removegrpBtn, SIGNAL( clicked() ), this, SLOT( removeGroupClicked() ) );
    connect( d->ui.activeCombo, SIGNAL(currentIndexChanged(int)),
             this, SLOT( activeGroupChanged(int)) );

    d->ui.deleteButton->setEnabled( false );
    d->ui.processDefaultButton->setEnabled( false );

    // setup process default environment map.
    QStringList procDefaultList = QProcess::systemEnvironment();
    foreach( QString _line, procDefaultList )
    {
        QString varName = _line.section( '=', 0, 0 );
        QString varValue = _line.section( '=', 1 );
        d->m_procDefaultMaps.insert( varName, varValue );
    }
}

EnvWidget::~EnvWidget()
{}

void EnvWidget::setConfig( KConfig *config, const QString &group )
{
    d->m_config = config;
    d->m_confgroup = group;
//     d->m_entry = entry;
}

void EnvWidget::setVariables(const QStringList& items)
{
    kDebug () << "Setting items " << items << endl;
    d->m_maps.clear();
    d->ui.activeCombo->clear();
    d->ui.variableTable->clear();

    foreach( QString _oneItem, items )
    {
        QString leftValue = _oneItem.section( '=', 0, 0 );
        QString varValue  = _oneItem.section( '=', 1 );

        QString group = leftValue.section( '_', 0, 0 );
        QString varName = leftValue.section( '_', 1 );

        d->m_maps[group].insert( varName, varValue );
    }

    // fill combobox with maps.key()
    // select one active item in combobox with 'enabled'
    disconnect( d->ui.activeCombo, SIGNAL(currentIndexChanged(int)),
                this, SLOT( activeGroupChanged(int)) );
    QList<QString> groupList = d->m_maps.keys();
//     if( groupList.count() > 0 )
//     {
    d->ui.activeCombo->addItems( groupList );
    if( !groupList.contains( d->m_enabled ) )
    {
        d->ui.activeCombo->addItem( d->m_enabled );
    }
    int idx = d->ui.activeCombo->findText( d->m_enabled );
    d->ui.activeCombo->setCurrentIndex( idx );
    activeGroupChanged( idx );
//     }

    connect( d->ui.activeCombo, SIGNAL(currentIndexChanged(int)),
             this, SLOT( activeGroupChanged(int)) );

}

QStringList EnvWidget::variables() const
{
    // convert d->m_maps into QList< GROUPNAME_VARNAME=VARVALUE >
    QStringList ret;

    QMap< QString, QMap<QString, QString> >::iterator it;
    for( it=d->m_maps.begin(); it!=d->m_maps.end(); ++it )
    {
        QString group = it.key();
        QMap<QString, QString> map = it.value();
        QMap<QString, QString>::iterator it2;
        for( it2=map.begin(); it2!=map.end(); ++it2 )
        {
            QString oneLine = group + '_' + it2.key() + '=' + it2.value();
            ret << oneLine;
        }
    }

    kDebug() << "Returning " << ret << endl;
    return ret;
}

void EnvWidget::loadSettings()
{
    KConfigGroup group( d->m_config, d->m_confgroup );
    d->m_enabled = group.readEntry( "Default Env Group", "default" );
    if( d->m_enabled.isEmpty() )
        d->m_enabled = QString( "default" );
    QStringList variables = group.readEntry( "Environment Variables", QStringList() );
    setVariables( variables );
}

void EnvWidget::saveSettings()
{
//     //make sure the maps are set
//     generateCurrentMaps();
//     Core::environment() ->saveSettings( m_currentOverrides );

    KConfigGroup envGroup( d->m_config, d->m_confgroup );
    envGroup.writeEntry( "Default Env Group", d->m_enabled );
    envGroup.writeEntry( "Environment Variables", this->variables() );
//     envGroup.config()->sync();
    envGroup.sync();
}

void EnvWidget::defaults()
{
//     load( true );
    loadSettings();
}

// Note: static method.
QStringList EnvWidget::environmentProfiles( KConfig *config )
{
    KConfigGroup cfg( config, "Project Env Settings" );
    QStringList ret;
    QStringList pairs = cfg.readEntry( "Environment Variables", QStringList() );
    foreach( QString _line, pairs )
    {
        QString leftValue = _line.section( '=', 0, 0 );
        QString group = leftValue.section( '_', 0, 0 );
        if( !ret.contains(group) )
            ret << group;
    }

    kDebug() << k_funcinfo << " Returning " << ret;
    return ret;
}

// Note: static method.
QMap<QString, QString> EnvWidget::environmentPairFor( KConfig *config,
        const QString &profile )
{
    KConfigGroup cfg( config, "Project Env Settings" );
    return EnvWidget::Private::environmentPairForPvt( cfg, profile );
}

// Note: static method.
QMap<QString, QString> EnvWidget::environmentPairFor( KSharedConfigPtr config,
        const QString &profile )
{
    KConfigGroup cfg( config, "Project Env Settings" );
    return EnvWidget::Private::environmentPairForPvt( cfg, profile );
}



// void EnvWidget::load( bool defaults )
// {
//     d->ui.variableTable->blockSignals( true );
//
//     //Clear the maps
//     if ( !defaults )
//     {
//         m_overrides.clear();
//     }
//     m_processDefaults.clear();
//     m_currentOverrides.clear();
//     m_currentProcessDefaults.clear();
//
//     //Clear the table widget
//     d->ui.variableTable->clearContents();
//     d->ui.variableTable->setRowCount( 0 );
//
//     if ( !defaults )  //Don't use overrides if only showing defaults
//     {
//         int i = 0;
//         QMap<QString, QString> ovrMap = Core::environment() ->overrideMap();
//         QMap<QString, QString>::const_iterator it = ovrMap.constBegin();
//         for ( ; it != ovrMap.constEnd(); ++it )
//         {
//             d->ui.variableTable->insertRow( i );
//
//             QTableWidgetItem * name = new QTableWidgetItem( it.key() );
//             setOverride( name );
//             d->ui.variableTable->setItem( i, 0, name );
//             name->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
//
//             QTableWidgetItem * value = new QTableWidgetItem( it.value() );
//             setOverride( value );
//             d->ui.variableTable->setItem( i, 1, value );
//
//             //Store this in a map for easy diffs
//             m_overrides.insert( it.key(), it.value() );
//             i++;
//         }
//     }
//
//     int i2 = 0;
//     QMap<QString, QString> proMap = Core::environment() ->processDefaultMap();
//     QMap<QString, QString>::const_iterator it2 = proMap.constBegin();
//     for ( ; it2 != proMap.constEnd(); ++it2 )
//     {
//         if ( Core::environment() ->isOverride( it2.key() ) )
//             continue;
//
//         d->ui.variableTable->insertRow( i2 );
//
//         QTableWidgetItem * name = new QTableWidgetItem( it2.key() );
//         setProcessDefault( name );
//         d->ui.variableTable->setItem( i2, 0, name );
//         name->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
//
//         QTableWidgetItem * value = new QTableWidgetItem( it2.value() );
//         setProcessDefault( value );
//         d->ui.variableTable->setItem( i2, 1, value );
//
//         //Store this in a map for easy diffs
//         m_processDefaults.insert( it2.key(), it2.value() );
//         i2++;
//     }
//
//     //FIXME This is a bug in Qt... #123207
//     //     d->ui.variableTable->setSortingEnabled( true );
//     //     d->ui.variableTable->sortItems( 0, Qt::DescendingOrder );
//     d->ui.variableTable->blockSignals( false );
//
//     if ( defaults )
//     {
//         emit changed( m_overrides.count() );
//     }
// }

void EnvWidget::newButtonClicked()
{
    KDialog * dialog = new KDialog( this );
    dialog->setCaption( i18n( "New Environment Variable" ) );
    dialog->setButtons( KDialog::Ok | KDialog::Cancel );
    dialog->setDefaultButton( KDialog::Ok );

    QWidget *main = new QWidget( dialog );
    QVBoxLayout *layout = new QVBoxLayout( main );

    layout->addWidget( new QLabel( i18n( "Name:" ), main ) );
    KLineEdit *nameEdit = new KLineEdit( main );
    layout->addWidget( nameEdit );
    layout->addWidget( new QLabel( i18n( "Value:" ), main ) );
    KTextEdit *valueEdit = new KTextEdit( main );
    layout->addWidget( valueEdit, 1 );
    nameEdit->setFocus();
    dialog->setMainWidget( main );

    if ( dialog->exec() == QDialog::Accepted )
    {
        QString _name = nameEdit->text();
        QString _value = valueEdit->toPlainText();
        if ( _name.isEmpty() )
            return ; //message box?

//         generateCurrentMaps();
//         if ( d->m_currentOverrides.contains( _name )
//              || d->m_currentProcessDefaults.contains( _name ) )
//             return ; //message box?

        // If no default group, create group named with 'default'
        if( d->ui.activeCombo->count() < 1 )
        {
            d->ui.activeCombo->addItem( "default" );
            d->m_enabled = "default";
        }

        // Add it to QMap
        QString currentGroup = d->ui.activeCombo->currentText();
        d->m_maps[ currentGroup ].insert( _name, _value );

        d->ui.variableTable->blockSignals( true );

        // Add it at the top?
        int row = d->ui.variableTable->rowCount();
        d->ui.variableTable->insertRow( row );

        QTableWidgetItem * name = new QTableWidgetItem( _name );
        setOverride( name );
        d->ui.variableTable->setItem( row, 0, name );
        name->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );

        QTableWidgetItem * value = new QTableWidgetItem( _value );
        setOverride( value );
        d->ui.variableTable->setItem( row, 1, value );

        //Make sure it is visible
        d->ui.variableTable->scrollToItem( name );

        d->ui.variableTable->blockSignals( false );

        d->ui.variableTable->setCurrentItem( name );

        d->ui.deleteButton->setEnabled( true );
        if( diff() )
            emit changed();
    }
}

void EnvWidget::deleteButtonClicked()
{
    QString curGroup = d->ui.activeCombo->currentText();
    if( curGroup.isEmpty() )
    {
        return;
    }

    int row = d->ui.variableTable->currentRow();
    if( d->m_maps.contains( curGroup ) )
    {

        QTableWidgetItem *item = d->ui.variableTable->item( row, 0 );
        if( !item ) return;
        d->m_maps[ curGroup ].remove( item->text() );
    }

    d->ui.variableTable->removeRow( row );
    d->ui.processDefaultButton->setEnabled( false );
    if( d->ui.variableTable->rowCount() < 1 )
        d->ui.deleteButton->setEnabled( false );
    if( diff() )
        emit changed();
}

void EnvWidget::processDefaultButtonClicked()
{
    // TODO setup process default in other ways
    int row = d->ui.variableTable->currentRow();
    QTableWidgetItem * name = d->ui.variableTable->item( row, 0 );
    QTableWidgetItem * value = d->ui.variableTable->item( row, 1 );
    if( name==0 || value==0 )
        return;
    QString _name = name->text();

    if( !d->m_procDefaultMaps.contains( _name ) )
        return;

//     QString pValue = Core::environment() ->processDefault( _name );
    QString pValue = d->m_procDefaultMaps[_name];

    value->setText( pValue );
    setProcessDefault( name );
    setProcessDefault( value );
    d->ui.processDefaultButton->setEnabled( false );
}

void EnvWidget::settingsChanged( int row, int /*column*/ )
{
    kDebug()<< k_funcinfo << endl;
    QTableWidgetItem * name = d->ui.variableTable->item( row, 0 );
    QTableWidgetItem * value = d->ui.variableTable->item( row, 1 );
    QString _name = name->text();
    QString _value = value->text();
    QString currentGroup = d->ui.activeCombo->currentText();

    (d->m_maps[ currentGroup ])[ _name ] = _value;

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

void EnvWidget::focusChanged( int row, int, int, int )
{
    if( row < 0 ) return;

    QTableWidgetItem *nameItem = d->ui.variableTable->item( row, 0 );
    QTableWidgetItem *valueItem = d->ui.variableTable->item( row, 1 );
    if( nameItem && valueItem )
    {
        QString name = nameItem->text();
        QString value = valueItem->text();

        // Var is processDefault and value is changed?
        bool pDefault = false;
        QString pValue;
        if( d->m_procDefaultMaps.contains(name) )
        {
            pDefault = true;
            QString pValue = d->m_procDefaultMaps[name];
        }
        d->ui.processDefaultButton->setEnabled( pDefault && pValue != value );

        // why do we disable delete button here?
    //     // Var is not processDefault?
    //     d->ui.deleteButton->setEnabled( !pDefault );
    }
}

void EnvWidget::addGroupClicked()
{
    QString curText = d->ui.activeCombo->currentText();
    if( d->m_maps.contains( curText ) )
    {
        return; // same group name cannot be added twice.
    }
//     d->m_enabled = curText;
//     d->ui.variableTable->clear();
    d->m_maps[curText];
    d->ui.activeCombo->addItem( curText ); // TODO test
    int idx = d->ui.activeCombo->findText( curText );
    activeGroupChanged( idx );
}

void EnvWidget::removeGroupClicked()
{
    int idx = d->ui.activeCombo->currentIndex();
    if( idx < 0 ) return;

    QString curText = d->ui.activeCombo->currentText();
    d->m_maps.remove( curText );
//     d->ui.variableTable->clear();
    d->ui.activeCombo->removeItem( idx );
    // Should call activeGroupChanged explicitly?
    // No. called by signal-slot automatically.
    activeGroupChanged( idx );
}

void EnvWidget::activeGroupChanged( int idx )
{
    int rowcount = d->ui.variableTable->rowCount();
    for( int i=0; i<rowcount; i++ )
        d->ui.variableTable->removeRow(0);

    if( idx == -1 )
    {
        // TODO combobox becomes empty or current index resetted.
        d->m_enabled = "";
    }
    else
    {
        d->m_enabled = d->ui.activeCombo->currentText();
        d->ui.variableTable->blockSignals(true);

        if( d->m_maps.contains( d->m_enabled ) )
        {
            // only sets enabled variable group into table
            QMap<QString, QString> map = d->m_maps[d->m_enabled];
            QMap<QString, QString>::iterator it;
            for( it=map.begin(); it!=map.end(); ++it )
            {
                //Add it at the top?
                int row = d->ui.variableTable->rowCount();
                d->ui.variableTable->insertRow( row );

                QTableWidgetItem * name = new QTableWidgetItem( it.key() );
                setOverride( name );
                d->ui.variableTable->setItem( row, 0, name );
                name->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );

                QTableWidgetItem * value = new QTableWidgetItem( it.value() );
                setOverride( value );
                d->ui.variableTable->setItem( row, 1, value );

                //Make sure it is visible
                d->ui.variableTable->scrollToItem( name );
            }
        }
        d->ui.variableTable->blockSignals(false);
    }
    if( d->ui.variableTable->rowCount() > 0 )
        d->ui.deleteButton->setEnabled( true );
    else
        d->ui.deleteButton->setEnabled( false );

    emit changed();
}

bool EnvWidget::isOverride( QTableWidgetItem *item ) const
{
    return ( item->textColor() == Qt::red );
}

bool EnvWidget::isProcessDefault( QTableWidgetItem *item ) const
{
    return ( item->textColor() == Qt::black );
}

void EnvWidget::setOverride( QTableWidgetItem *item )
{
    item->setTextColor( Qt::red );
}

void EnvWidget::setProcessDefault( QTableWidgetItem * item )
{
    item->setTextColor( Qt::black );
}

void EnvWidget::generateCurrentMaps()
{
    // TODO FIX
    //create maps of current settings to compare
//     m_currentOverrides.clear();
//     m_currentProcessDefaults.clear();
//     int rows = d->ui.variableTable->rowCount();
//     for ( int row = 0; row < rows; ++row )
//     {
//         QTableWidgetItem * name = d->ui.variableTable->item( row, 0 );
//         QTableWidgetItem * value = d->ui.variableTable->item( row, 1 );
//         QString _name = name->text();
//         QString _value = value->text();
//         if ( isOverride( name ) )
//             m_currentOverrides.insert( _name, _value );
//         else if ( isProcessDefault( name ) )
//             m_currentProcessDefaults.insert( _name, _value );
//     }
}

bool EnvWidget::diff()
{
    // TODO FIX
    return true;
//     generateCurrentMaps();
//     return ( m_currentOverrides != m_overrides
//              || m_currentProcessDefaults != m_processDefaults );
}

#include "envwidget.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
