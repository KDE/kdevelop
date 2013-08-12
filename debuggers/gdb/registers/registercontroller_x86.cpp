/*
 * Class to fetch/change/send registers to the debugger for x86, x86_64 architectures.
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

#include "registercontroller_x86.h"

#include "../debugsession.h"
#include "../gdbcommand.h"
#include "../mi/gdbmi.h"

#include <qmath.h>
#include <QRegExp>

#include <KDebug>

namespace GDBDebugger {

FlagRegister RegisterControllerGeneral_x86::m_eflags;

RegistersGroup&  RegisterControllerGeneral_x86::fillValuesForRegisters ( RegistersGroup& registersArray )
{
     kDebug() << "Filling values for registers: " << registersArray.groupName;
     if ( registersArray.groupName == enumToString ( Flags ) ) {
          registersArray = fillFlags ( registersArray );
     } else {
          registersArray = IRegisterController::fillValuesForRegisters ( registersArray );
     }

     return registersArray;
}

RegistersGroup& RegisterControllerGeneral_x86::getRegistersFromGroupInternally ( const QString& group )
{
     static RegistersGroup FPUregisters;
     static RegistersGroup FlagRegisters;
     static RegistersGroup SegmentRegisters;

     static bool initialized = false;

     if ( !initialized ) {
          QStringList l;

          FPUregisters.groupName = enumToString ( FPU );
          for ( int i = 0; i < 8; i++ ) {
               FPUregisters.registers.push_back ( Register ( "st" + QString::number ( i ), QString() ) );
          }

          m_eflags.flags << "C" << "P" << "A" << "Z" << "S" << "T" << "D" << "O";
          m_eflags.bits << "0" << "2" << "4" << "6" << "7" << "8" << "10" << "11";
          m_eflags.registerName = "eflags";

          FlagRegisters.groupName = enumToString ( Flags );
          FlagRegisters.flag = true;
          for ( int i = 0; i < m_eflags.flags.size(); i++ ) {
               FlagRegisters.registers.push_back ( Register ( m_eflags.flags[i], QString() ) );
          }

          SegmentRegisters.groupName = enumToString ( Segment );
          l.clear();
          l << "cs" << "ss" << "ds" << "es" << "fs" << "gs";
          foreach ( QString s, l ) {
               SegmentRegisters.registers.push_back ( Register ( s, QString() ) );
          }

          initialized = true;
     }

     if ( group == enumToString ( Flags ) ) {
          return FlagRegisters;
     } else if ( group == enumToString ( FPU ) ) {
          return FPUregisters;
     }  else if ( group == enumToString ( Segment ) ) {
          return SegmentRegisters;
     } else {
          kDebug() << group << "is incorrect group";
     }

     static RegistersGroup v;
     return v;
}
//Make some enum and grouptoString conversion.
const QStringList& RegisterControllerGeneral_x86::getNamesOfRegisterGroups() const
{
     static QStringList registerGroups;
     static bool initialized = false;
     if ( !initialized ) {
          registerGroups << enumToString ( General ) << enumToString ( Flags ) << enumToString ( FPU ) << enumToString ( XMM ) << enumToString ( Segment );
          initialized = true;
     }
     return registerGroups;
}

const RegistersGroup& RegisterControllerGeneral_x86::fillFlags ( RegistersGroup& flagsGroup )
{

     kDebug() << "Filling flags";

//     foreach(Register r, flagsGroup.registers) {
//         kDebug() << r.name << ' ' << r.value << '\n';
//     }

     bool ok;
     int flagsValue = getRegisterValue ( m_eflags.registerName ).toInt ( &ok,16 );

     for ( int idx = 0; idx < m_eflags.flags.count(); idx++ ) {
          flagsGroup.registers[idx].value = ( ( flagsValue >> m_eflags.bits[idx].toInt() ) & 1 ) ? "1": "0";
     }

//     foreach(Register r, flagsGroup.registers) {
//         kDebug() << r.name << ' ' << r.value;
//     }

     return flagsGroup;
}

void RegisterControllerGeneral_x86::setRegisterValueForGroup ( const QString& group, const Register& reg )
{
     if ( group == enumToString ( General ) ) {
          setGeneralRegister ( reg, group );
     } else if ( group == enumToString ( Flags ) ) {
          setFlagRegister ( reg, m_eflags );
     } else if ( group == enumToString ( FPU ) ) {
          setFPURegister ( reg );
     } else if ( group == enumToString ( XMM ) ) {
          setXMMRegister ( reg );
     } else if ( group == enumToString ( Segment ) ) {
          setSegmentRegister ( reg );
     } else {
          kDebug() << group << "is incorrect group";
     }
}

void RegisterControllerGeneral_x86::setFPURegister ( const Register& reg )
{
     kDebug() << "Setting FPU register through setGeneralRegister";
     setGeneralRegister ( reg, enumToString ( FPU ) );
}

void RegisterControllerGeneral_x86::setXMMRegister ( const Register& reg )
{
     kDebug() << "Setting XMM register through setGeneralRegister";
     //TODO:
     setGeneralRegister ( reg, enumToString ( XMM ) );
}

void RegisterControllerGeneral_x86::setSegmentRegister ( const Register& reg )
{
     kDebug() << "Setting segment register through setGeneralRegister";
     setGeneralRegister ( reg, enumToString ( Segment ) );
}

//TODO:
const RegistersTooltipGroup& RegisterControllerGeneral_x86::getTooltipsForRegistersInGroup ( const QString& /*group*/ ) const
{
     static RegistersTooltipGroup v;
     return v;
}

void RegisterControllerGeneral_x86::updateRegisters ( const QString group )
{
     if ( m_debugSession && !m_debugSession->stateIsOn ( s_dbgNotStarted|s_shuttingDown ) ) {
          if ( !m_registerNamesInitialized ) {
               initializeRegisters();
               m_registerNamesInitialized = true;
          }

          if ( group != enumToString ( FPU ) ) {
               if ( group.isEmpty() ) {
                    QStringList groups = getNamesOfRegisterGroups();
                    groups.removeOne ( enumToString ( FPU ) );
                    foreach ( QString g, groups ) {
                         IRegisterController::updateRegisters ( g );
                    }
               } else {
                    IRegisterController::updateRegisters ( group );
               }
          }

               if ( group == enumToString ( FPU ) || group.isEmpty() ) {
                    if ( m_debugSession && !m_debugSession->stateIsOn ( s_dbgNotStarted|s_shuttingDown ) ) {
                        //TODO: use mi interface instead.
                         m_debugSession->addCommand (
                              new CliCommand ( GDBMI::NonMI, "info all-registers $st0 $st1 $st2 $st3 $st4 $st5 $st6 $st7", this, &RegisterControllerGeneral_x86::handleFPURegisters ) );
                    }
          }
     }
}

QString RegisterControllerGeneral_x86::enumToString ( const RegisterGroups group ) const
{
     switch ( group ) {
     case General:
          return "General";
     case Flags:
          return "Flags";
     case FPU:
          return "FPU";
     case XMM:
          return "XMM";
     case Segment:
          return "Segment";
     }
     return QString();
}

RegistersGroup& RegisterController_x86_64::getRegistersFromGroupInternally ( const QString& group )
{
     static RegistersGroup generalPurposeRegisters;
     static RegistersGroup XMMregisters;

     static bool initialized = false;

     if ( !initialized ) {
          QStringList l;

          generalPurposeRegisters.groupName = enumToString ( General );
          l << "rax" << "rbx" << "rcx" << "rdx" << "rsi" << "rdi" << "rbp" << "rsp" << "r8" << "r9" << "r10" << "r11" << "r12" << "r13" << "r14" << "r15" << "rip";
          foreach ( QString s, l ) {
               generalPurposeRegisters.registers.push_back ( Register ( s, QString() ) );
          }

          XMMregisters.groupName = enumToString ( XMM );
          XMMregisters.editable = false;
          for ( int i = 0; i < 16; i++ ) {
               XMMregisters.registers.push_back ( Register ( "xmm" + QString::number ( i ), QString() ) );
          }

          initialized = true;
     }

     if ( group == enumToString ( General ) ) {
          return generalPurposeRegisters;
     } else if ( group == enumToString ( XMM ) ) {
          return XMMregisters;
     } else {
          return RegisterControllerGeneral_x86::getRegistersFromGroupInternally ( group );
     }
}

RegistersGroup& RegisterController_x86::getRegistersFromGroupInternally ( const QString& group )
{
     static RegistersGroup generalPurposeRegisters;
     static RegistersGroup XMMregisters;

     static bool initialized = false;

     if ( !initialized ) {
          QStringList l;

          generalPurposeRegisters.groupName = enumToString ( General );
          l << "eax" << "ebx" << "ecx" << "edx" << "esi" << "edi" << "ebp" << "esp" << "eip";
          foreach ( QString s, l ) {
               generalPurposeRegisters.registers.push_back ( Register ( s, QString() ) );
          }

          XMMregisters.groupName = enumToString ( XMM );
          XMMregisters.editable = false;
          for ( int i = 0; i < 8; i++ ) {
               XMMregisters.registers.push_back ( Register ( "xmm" + QString::number ( i ), QString() ) );
          }

          initialized = true;
     }

     if ( group == enumToString ( General ) ) {
          return generalPurposeRegisters;
     } else if ( group == enumToString ( XMM ) ) {
          return XMMregisters;
     } else {
          return RegisterControllerGeneral_x86::getRegistersFromGroupInternally ( group );
     }
}

const RegistersGroup& RegisterControllerGeneral_x86::convertValuesForGroup ( RegistersGroup& registersGroup, RegistersFormat format )
{
     if ( format != Raw && format != Natural ) {
          if ( registersGroup.groupName == enumToString ( General ) || registersGroup.groupName == enumToString ( Segment ) ) {
               return convertValuesForGroupInternally ( registersGroup, format );
          }
     }
     return registersGroup;
}

void RegisterControllerGeneral_x86::handleFPURegisters ( const QStringList& record )
{
     QRegExp rx ( "^(st[0-8])\\s+((?:-?\\d+\\.?\\d+(?:e(\\+|-)\\d+)?)|(?:\\d+))$" );
     QVector<Register> registers;
     foreach ( QString s, record ) {
          if ( rx.exactMatch ( s ) ) {
               registers.push_back ( Register ( rx.cap ( 1 ), rx.cap ( 2 ) ) );
          }
     }

     if ( registers.size() != 8 ) {
          kDebug() << "can't parse FPU. Wrong format";
          kDebug() << record;
          kDebug() << "registers ";
          foreach ( Register r, registers ) {
               kDebug() << r.name << ' ' << r.value;
          }
     } else {
          foreach ( Register r, registers ) {
               if ( m_registers.contains ( r.name ) ) {
                    m_registers[r.name] = r.value;
               }
          }
          emit registersInGroupChanged ( enumToString ( FPU ) );
     }
}

}
