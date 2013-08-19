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

#ifndef REGISTERCONTROLLER_ARM_H
#define REGISTERCONTROLLER_ARM_H


#include "registercontroller.h"

namespace GDBMI {
struct ResultRecord;
}

namespace GDBDebugger {

class DebugSession;

class RegisterController_Arm : public IRegisterController {
public:

     RegisterController_Arm ( QObject* parent, DebugSession* debugSession = 0 );

     virtual QStringList namesOfRegisterGroups() const;

public slots:
     virtual void updateRegisters ( const QString& group = QString() );

protected:
     virtual RegistersGroup& registersFromGroupInternally ( const QString& group );

     virtual RegistersGroup convertValuesForGroup ( RegistersGroup& registersGroup, RegistersFormat format = Raw );
     virtual QStringList registerNamesForGroup ( const QString& group );

     virtual RegistersGroup& updateValuesForRegisters ( RegistersGroup& registers );

     virtual void setRegisterValueForGroup ( const QString& group, const Register& reg );

     RegistersGroup updateFlagValues ( RegistersGroup& flagsGroup );

     enum RegisterGroups {General, Flags, VFP_single, VFP_double, VFP_quad};

     QString enumToString ( const RegisterGroups group ) const;
     //None of functions below checks value for validity, if value is invalid updateRegistres() will restore the previous state.

     void setVFPS_Register ( const Register& reg );
     void setVFPD_Register ( const Register& reg );
     void setVFPQ_Register ( const Register& reg );

     //VFP registers workaround.
     void handleVFPSRegisters( const QStringList& record );

     QStringList m_VFP_singleregisterNames;
     QStringList m_flagRegisterNames;
     QStringList m_VFP_doubleRegisterNames;
     QStringList m_generalRegisterNames;
     QStringList m_VFP_quadRegisterNames;

private:
     void initRegisterNames();

     static FlagRegister m_cpsr;

     bool m_registerNamesInitialized;
};

}
#endif // REGISTERCONTROLLER_ARM_H
