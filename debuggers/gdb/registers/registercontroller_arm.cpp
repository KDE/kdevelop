/*
 * Class to fetch/change/send registers to the debugger for arm architecture.
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

#include "registercontroller_arm.h"

#include "../debugsession.h"
#include "../gdbcommand.h"
#include "../mi/gdbmi.h"

#include <qmath.h>

#include <KDebug>

namespace GDBDebugger {

RegistersGroup&  RegisterController_Arm::updateValuesForRegisters ( RegistersGroup& registers )
{
     kDebug() << "Updating values for registers: " << registers.groupName;
     if ( registers.groupName == enumToString ( Flags ) ) {
          registers = updateFlagValues ( registers );
     } else {
          registers = IRegisterController::updateValuesForRegisters ( registers );
     }

     return registers;
}

RegistersGroup RegisterController_Arm::registersFromGroupInternally ( const QString& group )
{
     RegistersGroup registers;

     registers.groupName = group;
     registers.editable = ( group == enumToString ( VFP_double ) || group == enumToString ( VFP_quad ) ) ? false : true;
     registers.flag = ( group == enumToString ( Flags ) ) ? true : false;
     foreach ( QString name, registerNamesForGroup ( group ) ) {
          registers.registers.append ( Register ( name, QString() ) );
     }

     return updateValuesForRegisters ( registers );
}

QStringList RegisterController_Arm::namesOfRegisterGroups() const
{
     QStringList registerGroups;
     registerGroups << enumToString ( General ) << enumToString ( Flags ) << enumToString ( VFP_single ) << enumToString ( VFP_double ) << enumToString ( VFP_quad );

     return registerGroups;
}

RegistersGroup& RegisterController_Arm::updateFlagValues ( RegistersGroup& flagsGroup )
{
     kDebug() << "Updating flags";

     bool ok;
     quint32 flagsValue = registerValue ( m_cpsr.registerName ).toUInt ( &ok,16 );

     for ( int idx = 0; idx < m_cpsr.flags.count(); idx++ ) {
          flagsGroup.registers[idx].value = ( ( flagsValue >> m_cpsr.bits[idx].toInt() ) & 1 ) ? "1": "0";
     }

//     foreach(Register r, flagsGroup.registers) {
//         kDebug() << r.name << ' ' << r.value;
//     }

     return flagsGroup;
}

void RegisterController_Arm::setRegisterValueForGroup ( const QString& group, const Register& reg )
{
     if ( group == enumToString ( General ) ) {
          setGeneralRegister ( reg, enumToString ( General ) );
     } else if ( group == enumToString ( Flags ) ) {
          setFlagRegister ( reg, m_cpsr );
     } else if ( group == enumToString ( VFP_single ) ) {
          setVFPS_Register ( reg );
     } else if ( group == enumToString ( VFP_double ) ) {
          setVFPD_Register ( reg );
     } else if ( group == enumToString ( VFP_quad ) ) {
          setVFPQ_Register ( reg );
     } else {
          kDebug() << group << "is incorrect group";
     }
}

void RegisterController_Arm::setVFPS_Register ( const Register& reg )
{
     kDebug() << "Setting VFPS register through setGeneralRegister";
     setGeneralRegister ( reg, enumToString ( VFP_single ) );
}

void RegisterController_Arm::setVFPD_Register ( const Register& /*reg*/ )
{
     kDebug() << "Setting VFPD register through setGeneralRegister";
     //FIXME: gdb doesn't support setting values for registers with more than 64 bits directly(all that structures stuff should be used.).
     //setGeneralRegister ( reg, enumToString ( VFP_double ) );
}

void RegisterController_Arm::setVFPQ_Register ( const Register& /*reg */ )
{
     kDebug() << "Setting VFPQ register through setGeneralRegister";
     //FIXME:
     // setGeneralRegister ( reg, enumToString ( VFP_quad ) );
}

void RegisterController_Arm::updateRegisters ( const QString& group )
{
     if ( !m_debugSession || m_debugSession->stateIsOn ( s_dbgNotStarted|s_shuttingDown ) ) {
          return;
     }

     if ( !m_registerNamesInitialized ) {
          initializeRegisters();
          m_registerNamesInitialized = true;
     }

     if ( group != enumToString ( VFP_single ) ) {
          if ( group.isEmpty() ) {
               QStringList groups = namesOfRegisterGroups();
               groups.removeOne ( enumToString ( VFP_single ) );
               foreach ( const QString g, groups ) {
                    IRegisterController::updateRegisters ( g );
               }
          } else {
               IRegisterController::updateRegisters ( group );
          }
     }

     if ( group == enumToString ( VFP_single ) || group.isEmpty() ) {
          QString command = "info all-registers ";
          foreach ( const QString name, registerNamesForGroup ( enumToString ( VFP_single ) ) ) {
               command += "$" + name + ' ';
          }

          if ( m_debugSession && !m_debugSession->stateIsOn ( s_dbgNotStarted|s_shuttingDown ) ) {
               //TODO: use mi interface instead.
               m_debugSession->addCommand (
                    new CliCommand ( GDBMI::NonMI, command, this, &RegisterController_Arm::handleVFPSRegisters ) );
          }

          if ( group == enumToString ( VFP_single ) ) {
               IRegisterController::updateRegisters ( enumToString ( VFP_double ) );
               IRegisterController::updateRegisters ( enumToString ( VFP_quad ) );
          }
     }
}

void RegisterController_Arm::handleVFPSRegisters ( const QStringList& record )
{
     const QRegExp rx ( "^(s\\d+)\\s+((?:-?\\d+\\.?\\d+(?:e(\\+|-)\\d+)?)|(?:\\d+))$" );
     QVector<Register> registers;
     foreach ( const QString s, record ) {
          if ( rx.exactMatch ( s ) ) {
               registers.push_back ( Register ( rx.cap ( 1 ), rx.cap ( 2 ) ) );
          }
     }

     if ( registers.size() != 32 ) {
          kDebug() << "can't parse VFP single. Wrong format";
          kDebug() << record;
          kDebug() << "Parsed registers: ";
          foreach ( Register r, registers ) {
               kDebug() << r.name << ' ' << r.value;
          }
     } else {
          foreach ( const Register r, registers ) {
               if ( m_registers.contains ( r.name ) ) {
                    m_registers[r.name] = r.value;
               }
          }
          emit registersInGroupChanged ( enumToString ( VFP_single ) );
     }
}

QString RegisterController_Arm::enumToString ( const RegisterGroups& group ) const
{
     switch ( group ) {
     case General:
          return "General";
     case Flags:
          return "Flags";
     case VFP_single:
          return "VFP single-word";
     case VFP_double:
          return "VFP double-word";
     case VFP_quad:
          return "VFP quad-word";
     }
     return QString();
}

RegistersGroup& RegisterController_Arm::convertValuesForGroup ( RegistersGroup& registersGroup, const RegistersFormat& format )
{
     if ( format != Raw && format != Natural ) {
          if ( registersGroup.groupName == enumToString ( General ) ) {
               return IRegisterController::convertValuesForGroup ( registersGroup, format );
          }
     }

     return registersGroup;
}

RegisterController_Arm::RegisterController_Arm ( QObject* parent, DebugSession* debugSession ) :IRegisterController ( parent, debugSession ), m_registerNamesInitialized ( false )
{
     initRegisterNames();
}

void RegisterController_Arm::initRegisterNames()
{
     for ( int i = 0; i < 32; i++ ) {
          m_VFP_singleRegisterNames << ( "s" + QString::number ( i ) );
     }

     m_cpsr.registerName = "cpsr";
     m_cpsr.flags << "Q" << "V" << "C" << "Z" << "N";
     m_cpsr.bits << "27" << "28" << "29" << "30" << "31";

     m_flagRegisterNames = m_cpsr.flags;

     for ( int i = 0; i < 13; i++ ) {
          m_generalRegisterNames << ( "r" + QString::number ( i ) );
     }
     m_generalRegisterNames << "sp" << "lr" << "pc";

     for ( int i = 0; i < 32; i++ ) {
          m_VFP_doubleRegisterNames << ( "d" + QString::number ( i ) );
     }

     for ( int i = 0; i < 16; i++ ) {
          m_VFP_quadRegisterNames << ( "q" + QString::number ( i ) );
     }
}

QStringList RegisterController_Arm::registerNamesForGroup ( const QString& group )
{
     if ( group == enumToString ( General ) ) {
          return m_generalRegisterNames;
     } else if ( group == enumToString ( Flags ) ) {
          return m_flagRegisterNames;
     } else if ( group == enumToString ( VFP_single ) ) {
          return m_VFP_singleRegisterNames;
     } else if ( group == enumToString ( VFP_double ) ) {
          return m_VFP_doubleRegisterNames;
     } else if ( group == enumToString ( VFP_quad ) ) {
          return m_VFP_quadRegisterNames;
     } else {
          return QStringList();
     }
}

}
