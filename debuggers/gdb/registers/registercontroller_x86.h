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

#ifndef REGISTERCONTROLLER_X86_H
#define REGISTERCONTROLLER_X86_H

#include "registercontroller.h"

namespace GDBMI {
struct ResultRecord;
}

namespace GDBDebugger {
class DebugSession;
}

namespace GDBDebugger {


class RegisterControllerGeneral_x86 : public IRegisterController {
public:

     virtual const RegistersTooltipGroup& getTooltipsForRegistersInGroup ( const QString& group ) const;

     virtual const QStringList& getNamesOfRegisterGroups() const;

public slots:
     virtual void updateRegisters ( const QString group = QString() );

protected:
     RegisterControllerGeneral_x86 ( QObject* parent, DebugSession* debugSession = 0 ) :IRegisterController ( parent, debugSession ), m_registerNamesInitialized ( false ) {}

     virtual ~RegisterControllerGeneral_x86() {}

     virtual RegistersGroup& getRegistersFromGroupInternally ( const QString& group );

     virtual const RegistersGroup& convertValuesForGroup ( RegistersGroup& registersGroup, RegistersFormat format = Raw );

     virtual RegistersGroup& fillValuesForRegisters ( RegistersGroup& registersArray );

     virtual void setRegisterValueForGroup ( const QString& group, const Register& reg );

     const RegistersGroup& fillFlags ( RegistersGroup& flagsGroup );

     enum RegisterGroups {General, Flags, FPU, XMM, Segment};

     QString enumToString ( const RegisterGroups group ) const;
     //None of functions below checks value for validity, if value is invalid updateRegistres() will restore the previous state.

     void setFPURegister ( const Register& reg );
     void setXMMRegister ( const Register& reg );
     void setSegmentRegister ( const Register& reg );
     void setEFfagRegister ( const Register& reg );

     static FlagRegister m_eflags;

     bool m_registerNamesInitialized;
};

class RegisterController_x86 : public RegisterControllerGeneral_x86 {

public:
     RegisterController_x86 ( QObject* parent, DebugSession* debugSession = 0 ) :RegisterControllerGeneral_x86 ( parent, debugSession ) {}

     ~RegisterController_x86() {}

private:
     virtual RegistersGroup& getRegistersFromGroupInternally ( const QString& group );
};

class RegisterController_x86_64 : public RegisterControllerGeneral_x86 {

public:
     RegisterController_x86_64 ( QObject* parent, DebugSession* debugSession = 0 ) :RegisterControllerGeneral_x86 ( parent, debugSession ) {}

     ~RegisterController_x86_64() {}
private:

     virtual RegistersGroup& getRegistersFromGroupInternally ( const QString& group );
};
}

#endif // REGISTERCONTROLLER_X86_H
