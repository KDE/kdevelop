/*
 * Displays registers.
 * Copyright 2013  Vlas Puhov <vlas.puhov@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "registersview.h"

#include <QMenu>
#include <QContextMenuEvent>

#include <KDebug>

#include "registercontroller.h"

namespace GDBDebugger {

RegistersView::RegistersView ( QWidget* p ) : QWidget ( p ), m_registerController ( 0 ), m_tablesManager ( this ), m_registersFormat ( Raw )
{
     kDebug() << "setup ui";
     setupUi ( this );

     m_menu = new QMenu ( this );

     connect ( m_menu, SIGNAL ( triggered ( QAction* ) ), this, SLOT ( menuTriggered ( QAction* ) ) );

     m_tablesManager.addTable ( Table ( thirdTable, thirdLabel ) );
     m_tablesManager.addTable ( Table ( secondTable, secondLabel ) );
     m_tablesManager.addTable ( Table ( firstTable, firstLabel ) );

     m_tablesManager.load();
}

void RegistersView::updateRegistersInTable ( Table tableName, const RegistersGroup& registersGroup )
{
     tableName.name->setText ( registersGroup.groupName );

     tableName.tableWidget->blockSignals ( true );

     tableName.tableWidget->setRowCount ( registersGroup.registers.size() );
     kDebug() << "rows " << tableName.tableWidget->rowCount();
     for ( int i = 0 ; i < registersGroup.registers.size(); ++i ) {
          QTableWidgetItem *newItem = new QTableWidgetItem ( registersGroup.registers[i].name );
          newItem->setFlags ( Qt::ItemIsEnabled );
          tableName.tableWidget->setItem ( i, RegisterName, newItem );

          newItem = new QTableWidgetItem ( registersGroup.registers[i].value );
          if ( registersGroup.flag || !registersGroup.editable ) {
               newItem->setFlags ( Qt::ItemIsEnabled );
          }
          tableName.tableWidget->setItem ( i, RegisterValue, newItem );
     }

     int width = tableName.tableWidget->columnWidth ( RegisterName ) + tableName.tableWidget->columnWidth ( RegisterValue ) + 10;
     tableName.tableWidget->setMinimumWidth ( width );

     tableName.tableWidget->blockSignals ( false );
}

void RegistersView::registerChangedInternally ( QTableWidgetItem* item )
{
     kDebug() << "Sending changed register";
     if ( item->column() != RegisterValue ) {
          return;
     }

     const QTableWidget* table = static_cast<const QTableWidget*> ( sender() );

     Register changedRegister ( table->item ( item->row(), RegisterName )->text(), item->text() );

     kDebug() << changedRegister.name << ' ' << changedRegister.value;
     emit registerChanged ( changedRegister );
}

void RegistersView::contextMenuEvent ( QContextMenuEvent* e )
{
     QStringList groups;
     if ( m_registerController ) {
          groups = m_registerController->getNamesOfRegisterGroups();
     }

     m_menu->clear();

     m_menu->addAction ( "Update" );

     QAction* a;
     QMenu* m = m_menu->addMenu ( "Show" );
     foreach ( QString group, groups ) {
          a = m->addAction ( group );
          a->setCheckable ( true );
          if ( !m_tablesManager.getTableForGroup ( group ).isNull() ) {
               a->setChecked ( true );
          } else if ( !m_tablesManager.numOfFreeTables() ) {
               a->setEnabled ( false );
          }
     }

     m = m_menu->addMenu ( "Format" );
     addItemToFormatSubmenu ( m, QString ( "Dec" ), ( int ) Decimal );
     addItemToFormatSubmenu ( m, QString ( "Hex" ), ( int ) Hexadecimal );
     addItemToFormatSubmenu ( m, QString ( "Raw" ), ( int ) Raw );
     addItemToFormatSubmenu ( m, QString ( "Oct" ), ( int ) Octal );
     addItemToFormatSubmenu ( m, QString ( "Bin" ), ( int ) Binary );

     if ( !m_menu->actions().isEmpty() ) {
          m_menu->exec ( e->globalPos() );
     }
}

void RegistersView::addItemToFormatSubmenu ( QMenu* m, QString name, int format )
{
     QAction* a = m->addAction ( name );
     a->setData ( format );
     if ( ( RegistersFormat ) format == m_registersFormat ) {
          a->setCheckable ( true );
          a->setChecked ( true );
     }
}

void RegistersView::menuTriggered ( QAction* group )
{

     if ( group->text() == "Hex" || group->text() == "Dec" || group->text() == "Oct" || group->text() == "Bin" || group->text() == "Raw" ) {
          m_registersFormat = group->data().toInt();
          if ( m_registerController ) {
               m_registerController->updateRegisters();
          }
     } else if ( group->text() == "Update" ) {
          if ( m_registerController ) {
               m_registerController->updateRegisters();
          }
     } else {
          kDebug() << "Changing table for group" << group->text();
          Table t;
          t = m_tablesManager.getTableForGroup ( group->text() );
          //already showing
          if ( !t.isNull() ) {
               kDebug() << "Already showing";
               m_tablesManager.removeAssociation ( group->text() );
          } else {
               kDebug() << "Create";
               t = m_tablesManager.createTableForGroup ( group->text() );
               if ( !t.isNull() ) {
                    kDebug() << "Update";
                    if ( m_registerController ) {
                         m_registerController->updateRegisters ( group->text() );
                    }
               }
          }
     }
}

RegistersView::Table RegistersView::TablesManager::getTableForGroup ( const QString& group ) const
{
     Table t;

     if ( !group.isEmpty() ) {
          foreach ( TableRegistersAssociation a, m_tableRegistersAssociation ) {
               if ( a.registersGroup == group ) {
                    kDebug() << "Get table" << group;
                    t = a.table;
                    break;
               }
          }
     }

     return t;
}

RegistersView::Table RegistersView::TablesManager::createTableForGroup ( const QString& group )
{
     Table t;
     if ( !group.isEmpty() ) {
          for ( int i = 0; i < m_tableRegistersAssociation.count(); i++ ) {
               if ( m_tableRegistersAssociation[i].registersGroup.isEmpty() ) {
                    kDebug() << "Create table for group" << group;
                    m_tableRegistersAssociation[i].registersGroup = group;
                    t = m_tableRegistersAssociation[i].table;
                    t.name->show();
                    t.tableWidget->show();
                    break;
               }
          }
     }

     return t;
}

bool RegistersView::TablesManager::removeAssociation ( const QString& group )
{
     if ( !group.isEmpty() ) {
          for ( int i = 0; i < m_tableRegistersAssociation.count(); i++ ) {
               if ( m_tableRegistersAssociation[i].registersGroup == group ) {
                    kDebug() << "Remove association " << group;
                    m_tableRegistersAssociation[i].registersGroup.clear();
                    m_tableRegistersAssociation[i].table.tableWidget->hide();
                    m_tableRegistersAssociation[i].table.name->hide();
                    return true;
               }
          }
     }
     return false;
}

void RegistersView::TablesManager::addTable ( RegistersView::Table table )
{
     kDebug() << "Add table" << table.name->text();
     m_tableRegistersAssociation.push_back ( TableRegistersAssociation ( table, "" ) );

     table.tableWidget->hide();
     table.name->hide();
     table.tableWidget->horizontalHeader()->setResizeMode ( QHeaderView::ResizeToContents );
     table.tableWidget->setSelectionMode ( QAbstractItemView::SingleSelection );
     table.tableWidget->setMinimumWidth ( 10 );

     connect ( table.tableWidget, SIGNAL ( itemChanged ( QTableWidgetItem* ) ), m_parent, SLOT ( registerChangedInternally ( QTableWidgetItem* ) ) );
     connect ( table.tableWidget, SIGNAL ( itemDoubleClicked ( QTableWidgetItem* ) ), m_parent, SLOT ( flagChangedInternally ( QTableWidgetItem* ) ) );
}

int RegistersView::TablesManager::numOfFreeTables() const
{
     int count = 0;
     foreach ( TableRegistersAssociation a, m_tableRegistersAssociation ) {
          if ( a.registersGroup.isEmpty() ) {
               count++;
          }
     }
     return count;
}

void RegistersView::registersInGroupChanged ( const QString& group )
{

     kDebug() << "Updating GUI";

     if ( m_registerController ) {
          const RegistersGroup& registersGroup = m_registerController->getRegistersFromGroup ( group, ( RegistersFormat ) m_registersFormat );

          Table t = m_tablesManager.getTableForGroup ( registersGroup.groupName );
          if ( !t.isNull() ) {
               updateRegistersInTable ( t, registersGroup );
          }
     }
}

void RegistersView::setController ( IRegisterController* controller )
{
     if ( controller && controller != m_registerController ) {

          connect ( controller, SIGNAL ( registersInGroupChanged ( QString ) ), this, SLOT ( registersInGroupChanged ( QString ) ) );

          connect ( this, SIGNAL ( registerChanged ( const Register& ) ), controller, SLOT ( setRegisterValue ( const Register& ) ) );

          //if architecture has changed, clear all tables.
          QStringList groups = controller->getNamesOfRegisterGroups();
          foreach ( QString g, m_tablesManager.getAllGroups() ) {
               if ( !groups.contains ( g ) ) {
                    m_tablesManager.clearAllAssociations();
                    m_tablesManager.createTableForGroup ( "General" );
                    break;
               }
          }
     }
     setEnabled ( controller ? true : false );
     m_registerController = controller;
}

const QStringList RegistersView::TablesManager::getAllGroups() const
{
     QStringList groups;
     foreach ( TableRegistersAssociation a, m_tableRegistersAssociation ) {
          if ( !a.registersGroup.isEmpty() ) {
               groups << a.registersGroup;
          }
     }
     return groups;
}

void RegistersView::TablesManager::save()
{
     m_config.writeEntry ( "format", m_parent->m_registersFormat );
     m_config.writeEntry ( "number", m_tableRegistersAssociation.count() - numOfFreeTables() );

     QStringList groups = getAllGroups();

     for ( int i = 0; i < groups.count(); i++ ) {
          kDebug() << "Saving group" << groups[i];
          m_config.writeEntry ( QString::number ( i ), groups[i] );
     }

}

void RegistersView::TablesManager::load()
{
     m_parent->m_registersFormat = m_config.readEntry ( "format", m_parent->m_registersFormat );
     int tablesCount = m_config.readEntry ( "number", -1 );
     if ( tablesCount == -1 ) {
          createTableForGroup ( "General" );
     } else {
          for ( int i = 0; i < tablesCount; i++ ) {
               createTableForGroup ( m_config.readEntry ( QString::number ( i ), QString() ) );
          }
     }
}

RegistersView::TablesManager::TablesManager ( RegistersView* parent ) :m_parent ( parent )
{
     m_config = KGlobal::config()->group ( "Tables manager" );
}

void RegistersView::TablesManager::clearAllAssociations()
{
     for ( int i = 0; i < m_tableRegistersAssociation.count(); i++ ) {
          removeAssociation ( m_tableRegistersAssociation[i].registersGroup );
     }
}

void RegistersView::flagChangedInternally ( QTableWidgetItem* item )
{
     if ( item->column() != RegisterValue || item->text().length() != 1 || item->flags() & Qt::ItemIsEditable ) {
          return;
     }

     const QTableWidget* table = static_cast<const QTableWidget*> ( sender() );

     Register changedRegister ( table->item ( item->row(), RegisterName )->text(), item->text() );

     kDebug() << changedRegister.name << ' ' << changedRegister.value;
     emit registerChanged ( changedRegister );
}

}
