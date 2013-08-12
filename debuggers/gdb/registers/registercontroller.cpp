/*
 * Class to fetch/change/send registers to the debugger.
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

#include "registercontroller.h"

#include "../debugsession.h"

#include <qmath.h>

#include <KDebug>

#include "../gdbcommand.h"
#include "../mi/gdbmi.h"

namespace GDBDebugger {

void IRegisterController::setSession ( DebugSession* debugSession )
{
     m_debugSession = debugSession;
}

void IRegisterController::updateRegisters ( const QString group )
{
     if ( m_pendingGroups.contains ( group ) ) {
          kDebug() << "Already updating " << group;
          return;
     }

     if ( group.isEmpty() ) {
          kDebug() << "Update all";
          m_pendingGroups.clear();
     } else {
          kDebug() << "Update" << group << "All groups: " << m_pendingGroups;
          m_pendingGroups << group;
          if ( m_pendingGroups.size() != 1 ) {
               return;
          }
     }

     if ( m_debugSession && !m_debugSession->stateIsOn ( s_dbgNotStarted|s_shuttingDown ) ) {
          m_debugSession->addCommand (
               new GDBCommand ( GDBMI::DataListRegisterValues, "r", this, &IRegisterController::updateRegisterValuesHandler ) );
     }
}

void IRegisterController::getRegisterNamesHandler ( const GDBMI::ResultRecord& r )
{
     const GDBMI::Value& names = r["register-names"];

     m_rawRegisterNames.clear();
     for ( int i = 0; i < names.size(); ++i ) {
          const GDBMI::Value& entry = names[i];
          if ( !entry.literal().isEmpty() ) {
               //  kDebug() << "Inserting name " << entry.literal() << "number " << i;
               m_rawRegisterNames.insert ( i, entry.literal() );
          }
     }
}

void IRegisterController::updateRegisterValuesHandler ( const GDBMI::ResultRecord& r )
{
     if ( m_rawRegisterNames.isEmpty() ) {
          kDebug() << "Registers not initialized yet!";
          return;
     }

     const GDBMI::Value& values = r["register-values"];
     for ( int i = 0; i < values.size(); ++i ) {
          const GDBMI::Value& entry = values[i];
          int number = entry["number"].literal().toInt();
          //kDebug() << "Considering number " << number;
          if ( m_rawRegisterNames.contains ( number ) ) {
               QString value = entry["value"].literal();
               m_registers.insert ( m_rawRegisterNames[number], value );
               //  kDebug() << "Inserting value " << v << "to name" << m_registres[number].name;
          }
     }
     kDebug() << m_pendingGroups.size() << "groups to change registers";
     kDebug() << m_pendingGroups;
     foreach ( QString group, getNamesOfRegisterGroups() ) {
          if ( m_pendingGroups.isEmpty() || m_pendingGroups.contains ( group ) ) {
               emit registersInGroupChanged ( group );
          }
     }
     m_pendingGroups.clear();
}

void IRegisterController::setRegisterValue ( const Register& reg )
{

     if ( !m_rawRegisterNames.isEmpty() ) {
          QString group = getGroupForRegisterName ( reg.name );
          if ( !group.isEmpty() ) {
               setRegisterValueForGroup ( group, reg );
          }
     } else {
          kDebug() << "Registers not initialized yet!";
     }
}

const QString& IRegisterController::getRegisterValue ( const QString& name ) const
{
     Q_ASSERT ( !m_rawRegisterNames.isEmpty() );
     static QString value;
     if ( !name.isEmpty() ) {
          if ( m_registers.contains ( name ) ) {
               value = m_registers.value ( name );
          }
     }
     return value;
}

void IRegisterController::initializeRegisters()
{
     m_debugSession->addCommand (
          new GDBCommand ( GDBMI::DataListRegisterNames, "", this, &IRegisterController::getRegisterNamesHandler ) );
}

const QString IRegisterController::getGroupForRegisterName ( const QString& name )
{
     RegistersGroup registersInGroups;

     foreach ( QString group, getNamesOfRegisterGroups() ) {
          registersInGroups = getRegistersFromGroupInternally ( group );
          for ( int i = 0 ; i < registersInGroups.registers.count(); ++i ) {
               if ( registersInGroups.registers[i].name == name ) {
                    return group;
               }
          }
     }
     return QString();
}

const RegistersGroup& IRegisterController::getRegistersFromGroup ( const QString& group, const RegistersFormat format )
{
     return convertValuesForGroup ( fillValuesForRegisters ( getRegistersFromGroupInternally ( group ) ), format );
}

RegistersGroup& IRegisterController::fillValuesForRegisters ( RegistersGroup& registersArray )
{
     if ( m_rawRegisterNames.isEmpty() ) {
          kDebug() << "Registers not initialized yet";
          static RegistersGroup empty;
          return empty;
     }

     for ( int i = 0; i < registersArray.registers.size(); i++ ) {
          if ( m_registers.contains ( registersArray.registers[i].name ) ) {
               registersArray.registers[i].value = m_registers.value ( registersArray.registers[i].name );
          }
     }
     return registersArray;
}

void IRegisterController::setFlagRegister ( const Register& reg, const FlagRegister& flag )
{
     bool ok;
     quint32 flagsValue = getRegisterValue ( flag.registerName ).toUInt ( &ok,16 );

     kDebug() << "Set flag " << reg.name << ' ' << reg.value << ' ' << flag.flags << flagsValue;
     const int idx = flag.flags.indexOf ( reg.name );

     if ( idx != -1 ) {
          flagsValue ^= ( int ) qPow ( 2, flag.bits[idx].toUInt() );
          setGeneralRegister ( Register ( flag.registerName, QString ( "%1" ).arg ( flagsValue ) ) , "Flags" );
     } else {
          updateRegisters ( "Flags" );
          kDebug() << reg.name << ' ' << reg.value << "is incorrect flag name/value";
     }
}

void IRegisterController::setGeneralRegister ( const Register& reg, const QString& group )
{
     QString command = QString ( "set var $%1=%2" ).arg ( reg.name ).arg ( reg.value );
     kDebug() << "Setting general register" << command;

     if ( m_debugSession ) {
          m_debugSession->addCommand ( new GDBCommand ( GDBMI::NonMI, command ) );
          updateRegisters ( group );
     } else {
          kDebug() << "Session has ended";
     }
}

const RegistersGroup& IRegisterController::convertValuesForGroup ( RegistersGroup& registersGroup, RegistersFormat /*format*/ )
{
     return registersGroup;
}

const RegistersGroup& IRegisterController::convertValuesForGroupInternally ( RegistersGroup& registersGroup, RegistersFormat format )
{
     kDebug() << "Converting for group" << registersGroup.groupName;
     bool ok;
     for ( int i = 0; i < registersGroup.registers.size(); i++ ) {
          QString converted = QString::number ( registersGroup.registers[i].value.toULongLong ( &ok, 16 ), ( int ) format );
          // kDebug() << "Before " << registersGroup.registers[i].value;
          if ( ok ) {
               registersGroup.registers[i].value = converted;
          }
          // kDebug() << "After " << registersGroup.registers[i].value;
     }

     return registersGroup;
}

}
