/*
 * Creates RegistersView and RegisterController based on current architecture.
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

#include "registersmanager.h"

#include "registercontroller_x86.h"
#include "registercontroller_arm.h"
#include "registersview.h"

#include "modelsmanager.h"

#include "../gdbcommand.h"
#include "../debugsession.h"
#include "../debug.h"



namespace GDBDebugger
{

void ArchitectureParser::parseArchitecture()
{
    Architecture arch = other;

    foreach (const QString & reg, m_registerNames) {
        if (reg == "rax") {
            arch = x86_64;
            break;
        } else if (reg == "r0") {
            arch = arm;
            break;
        } else if (reg == "eax") {
            arch = x86;
            //we don't break because x86_64 contains eax too.
        }
    }

    emit architectureParsed(arch);
}

void ArchitectureParser::registerNamesHandler(const GDBMI::ResultRecord& r)
{
    const GDBMI::Value& names = r["register-names"];

    m_registerNames.clear();
    for (int i = 0; i < names.size(); ++i) {
        const GDBMI::Value& entry = names[i];
        if (!entry.literal().isEmpty()) {
            m_registerNames << entry.literal();
        }
    }

    parseArchitecture();
}

void ArchitectureParser::determineArchitecture(DebugSession* debugSession)
{
    if (!debugSession || debugSession->stateIsOn(s_dbgNotStarted | s_shuttingDown)) {
        return;
    }

    debugSession->addCommand(
        new GDBCommand(GDBMI::DataListRegisterNames, "", this, &ArchitectureParser::registerNamesHandler));
}

RegistersManager::RegistersManager(QWidget* parent)
: QObject(parent), m_registersView(new RegistersView(parent)), m_registerController(0), m_architectureParser(new ArchitectureParser(this)), m_debugSession(0), m_modelsManager(new ModelsManager(this)), m_currentArchitecture(undefined), m_needToCheckArch(false)
{
    connect(m_architectureParser, &ArchitectureParser::architectureParsed, this, &RegistersManager::architectureParsedSlot);

    m_registersView->setModel(m_modelsManager);
    setController(0);
}

void RegistersManager::architectureParsedSlot(Architecture arch)
{
    qCDebug(DEBUGGERGDB) << " Current controller: " << m_registerController << "Current arch " << m_currentArchitecture;

    if (m_registerController || m_currentArchitecture != undefined) {
        return;
    }

    switch (arch) {
    case x86:
        m_registerController.reset(new RegisterController_x86(m_debugSession)) ;
        qCDebug(DEBUGGERGDB) << "Found x86 architecture";
        break;
    case x86_64:
        m_registerController.reset(new RegisterController_x86_64(m_debugSession));
        qCDebug(DEBUGGERGDB) << "Found x86_64 architecture";
        break;
    case arm:
        m_registerController.reset(new RegisterController_Arm(m_debugSession));
        qCDebug(DEBUGGERGDB) << "Found Arm architecture";
        break;
    default:
        m_registerController.reset();
        qWarning() << "Unsupported architecture. Registers won't be available.";
        break;
    }

    m_currentArchitecture = arch;

    setController(m_registerController.data());

    if (m_registerController) {
        updateRegisters();
    }
}

void RegistersManager::setSession(DebugSession* debugSession)
{
    qCDebug(DEBUGGERGDB) << "Change session " << debugSession;
    m_debugSession = debugSession;
    if (m_registerController) {
        m_registerController->setSession(debugSession);
    }
    if (!m_debugSession) {
        qCDebug(DEBUGGERGDB) << "Will reparse arch";
        m_needToCheckArch = true;
        setController(0);
    }
}

void RegistersManager::updateRegisters()
{
    if (!m_debugSession || m_debugSession->stateIsOn(s_dbgNotStarted | s_shuttingDown)) {
        return;
    }

    qCDebug(DEBUGGERGDB) << "Updating registers";
    if (m_needToCheckArch) {
        m_needToCheckArch = false;
        m_currentArchitecture = undefined;
        setController(0);
    }
    if (m_currentArchitecture == undefined) {
        m_architectureParser->determineArchitecture(m_debugSession);
    }

    if (m_registerController) {
        m_registersView->updateRegisters();
    } else {
        qCDebug(DEBUGGERGDB) << "No registerController, yet?";
    }
}

ArchitectureParser::ArchitectureParser(QObject* parent)
: QObject(parent) {}

void RegistersManager::setController(IRegisterController* c)
{
    m_registerController.reset(c);
    m_modelsManager->setController(c);

    m_registersView->enable(c ? true : false);
}

}
