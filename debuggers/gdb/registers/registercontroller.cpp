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

void IRegisterController::updateRegisters ( const QString& group )
{
     if ( m_pendingGroups.contains ( group ) ) {
          kDebug() << "Already updating " << group;
          return;
     }

     if ( group.isEmpty() ) {
          kDebug() << "Update all";
          m_pendingGroups.clear();
     } else {
          kDebug() << "Update " << group << "All groups: " << m_pendingGroups;
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

void IRegisterController::registerNamesHandler ( const GDBMI::ResultRecord& r )
{
     const GDBMI::Value& names = r["register-names"];

     m_rawRegisterNames.clear();
     for ( int i = 0; i < names.size(); ++i ) {
          const GDBMI::Value& entry = names[i];
          //  kDebug() << "Inserting name " << entry.literal() << "number " << i;
          m_rawRegisterNames.push_back ( entry.literal() );
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
          Q_ASSERT ( m_rawRegisterNames.size() >  number );
          if ( !m_rawRegisterNames[number].isEmpty() ) {
               QString value = entry["value"].literal();
               m_registers.insert ( m_rawRegisterNames[number], value );
          }
     }

     kDebug() << "groups to change registers: " << m_pendingGroups;
     foreach ( const QString group, namesOfRegisterGroups() ) {
          if ( m_pendingGroups.isEmpty() || m_pendingGroups.contains ( group ) ) {
               emit registersInGroupChanged ( group );
          }
     }
     m_pendingGroups.clear();
}

void IRegisterController::setRegisterValue ( const Register& reg )
{

     if ( !m_rawRegisterNames.isEmpty() ) {
          const QString group = groupForRegisterName ( reg.name );
          if ( !group.isEmpty() ) {
               setRegisterValueForGroup ( group, reg );
          }
     } else {
          kDebug() << "Registers not initialized yet!";
     }
}

QString IRegisterController::registerValue ( const QString& name ) const
{
     Q_ASSERT ( !m_rawRegisterNames.isEmpty() );
     QString value;
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
          new GDBCommand ( GDBMI::DataListRegisterNames, "", this, &IRegisterController::registerNamesHandler ) );
}

QString IRegisterController::groupForRegisterName ( const QString& name )
{
     foreach ( const QString group, namesOfRegisterGroups() ) {
          RegistersGroup registersInGroups = registersFromGroupInternally ( group );
          foreach ( const Register r, registersInGroups.registers ) {
               if ( r.name == name ) {
                    return group;
               }
          }
     }
     return QString();
}

RegistersGroup IRegisterController::registersFromGroup ( const QString& group, const RegistersFormat& format )
{
     RegistersGroup g = registersFromGroupInternally ( group );
     return convertValuesForGroup ( g, format );
}

RegistersGroup& IRegisterController::updateValuesForRegisters ( RegistersGroup& registers )
{
     if ( m_rawRegisterNames.isEmpty() ) {
          kDebug() << "Registers not initialized yet";
          static RegistersGroup empty;
          return empty;
     }

     for ( int i = 0; i < registers.registers.size(); i++ ) {
          if ( m_registers.contains ( registers.registers[i].name ) ) {
               registers.registers[i].value = m_registers.value ( registers.registers[i].name );
          }
     }
     return registers;
}

void IRegisterController::setFlagRegister ( const Register& reg, const FlagRegister& flag )
{
     bool ok;
     quint32 flagsValue = registerValue ( flag.registerName ).toUInt ( &ok,16 );

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
     const QString command = QString ( "set var $%1=%2" ).arg ( reg.name ).arg ( reg.value );
     kDebug() << "Setting general register" << command;

     if ( m_debugSession ) {
          m_debugSession->addCommand ( new GDBCommand ( GDBMI::NonMI, command ) );
          updateRegisters ( group );
     } else {
          kDebug() << "Session has ended";
     }
}

RegistersGroup& IRegisterController::convertValuesForGroup ( RegistersGroup& registersGroup, const RegistersFormat& format )
{
     kDebug() << "Converting for group" << registersGroup.groupName;
     bool ok;
     for ( int i = 0; i < registersGroup.registers.size(); i++ ) {
          const QString converted = QString::number ( registersGroup.registers[i].value.toULongLong ( &ok, 16 ), ( int ) format );
          // kDebug() << "Before " << registersGroup.registers[i].value;
          if ( ok ) {
               registersGroup.registers[i].value = converted;
          }
          // kDebug() << "After " << registersGroup.registers[i].value;
     }

     return registersGroup;
}

IRegisterController::IRegisterController ( QObject* parent, DebugSession* debugSession ) :QObject ( parent ), m_debugSession ( debugSession ) {}

IRegisterController::~IRegisterController() {}

}
