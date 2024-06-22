/*
    SPDX-FileCopyrightText: 2013 Vlas Puhov <vlas.puhov@mail.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "registersmanager.h"

#include "registercontroller_arm.h"
#include "registercontroller_x86.h"
#include "registersview.h"

#include "dbgglobal.h"
#include "debuglog.h"
#include "midebugsession.h"
#include "mi/micommand.h"
#include "modelsmanager.h"

using namespace KDevMI::MI;
using namespace KDevMI;

void ArchitectureParser::parseArchitecture()
{
    Architecture arch = other;

    for (const QString& reg : qAsConst(m_registerNames)) {
        if (reg == QLatin1String("rax")) {
            arch = x86_64;
            break;
        } else if (reg == QLatin1String("r0")) {
            arch = arm;
            break;
        } else if (reg == QLatin1String("eax")) {
            arch = x86;
            //we don't break because x86_64 contains eax too.
        }
    }

    emit architectureParsed(arch);
}

void ArchitectureParser::registerNamesHandler(const ResultRecord& r)
{
    const Value& names = r[QStringLiteral("register-names")];

    m_registerNames.clear();
    for (int i = 0; i < names.size(); ++i) {
        const Value& entry = names[i];
        if (!entry.literal().isEmpty()) {
            m_registerNames << entry.literal();
        }
    }

    parseArchitecture();
}

void ArchitectureParser::determineArchitecture(MIDebugSession* debugSession)
{
    if (!debugSession || debugSession->debuggerStateIsOn(s_dbgNotStarted | s_shuttingDown)) {
        return;
    }

    debugSession->addCommand(DataListRegisterNames, QString(), this, &ArchitectureParser::registerNamesHandler);
}

RegistersManager::RegistersManager(QWidget* parent)
: QObject(parent), m_registersView(new RegistersView(parent)), m_registerController(nullptr), m_architectureParser(new ArchitectureParser(this)), m_debugSession(nullptr), m_modelsManager(new ModelsManager(this)), m_currentArchitecture(undefined), m_needToCheckArch(false)
{
    connect(m_architectureParser, &ArchitectureParser::architectureParsed, this, &RegistersManager::architectureParsedSlot);

    m_registersView->setModel(m_modelsManager);
    setController(nullptr);
}

void RegistersManager::architectureParsedSlot(Architecture arch)
{
    qCDebug(DEBUGGERCOMMON) << " Current controller: " << m_registerController.get() << "Current arch " << m_currentArchitecture;

    if (m_registerController || m_currentArchitecture != undefined) {
        return;
    }

    switch (arch) {
    case x86:
        m_registerController.reset(new RegisterController_x86(m_debugSession)) ;
        qCDebug(DEBUGGERCOMMON) << "Found x86 architecture";
        break;
    case x86_64:
        m_registerController.reset(new RegisterController_x86_64(m_debugSession));
        qCDebug(DEBUGGERCOMMON) << "Found x86_64 architecture";
        break;
    case arm:
        m_registerController.reset(new RegisterController_Arm(m_debugSession));
        qCDebug(DEBUGGERCOMMON) << "Found Arm architecture";
        break;
    default:
        m_registerController.reset();
        qCWarning(DEBUGGERCOMMON) << "Unsupported architecture. Registers won't be available.";
        break;
    }

    m_currentArchitecture = arch;

    setController(m_registerController.data());

    if (m_registerController) {
        updateRegisters();
    }
}

void RegistersManager::setSession(MIDebugSession* debugSession)
{
    qCDebug(DEBUGGERCOMMON) << "Change session " << debugSession;
    m_debugSession = debugSession;
    if (m_registerController) {
        m_registerController->setSession(debugSession);
    }
    if (!m_debugSession) {
        qCDebug(DEBUGGERCOMMON) << "Will reparse arch";
        m_needToCheckArch = true;
        setController(nullptr);
    }
}

void RegistersManager::updateRegisters()
{
    if (!m_debugSession || m_debugSession->debuggerStateIsOn(s_dbgNotStarted | s_shuttingDown)) {
        return;
    }

    qCDebug(DEBUGGERCOMMON) << "Updating registers";
    if (m_needToCheckArch) {
        m_needToCheckArch = false;
        m_currentArchitecture = undefined;
        setController(nullptr);
    }
    if (m_currentArchitecture == undefined) {
        m_architectureParser->determineArchitecture(m_debugSession);
    }

    if (m_registerController) {
        m_registersView->updateRegisters();
    } else {
        qCDebug(DEBUGGERCOMMON) << "No registerController, yet?";
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

#include "moc_registersmanager.cpp"
