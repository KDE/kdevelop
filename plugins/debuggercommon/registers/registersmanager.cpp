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

#include <QStringList>

using namespace KDevMI::MI;
using namespace KDevMI;

namespace {
enum Architecture {
    x86,
    x86_64,
    arm,
    other
};

[[nodiscard]] Architecture determineArchitecture(const QStringList& registerNames)
{
    Architecture arch = other;

    for (const auto& reg : registerNames) {
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

    return arch;
}

[[nodiscard]] QStringList parseRegisterNames(const ResultRecord& r)
{
    const Value& names = r[QStringLiteral("register-names")];
    const auto nameCount = names.size();

    QStringList registerNames;
    registerNames.reserve(nameCount);
    for (auto i = 0; i < nameCount; ++i) {
        const auto& literal = names[i].literal();
        if (!literal.isEmpty()) {
            registerNames.push_back(literal);
        }
    }
    return registerNames;
}

} // unnamed namespace

RegistersManager::RegistersManager(QWidget* parent)
    : QObject(parent)
    , m_modelsManager{new ModelsManager(this)}
    , m_registersView{new RegistersView(parent)}
{
    m_registersView->setModel(m_modelsManager);
    setController(nullptr);
}

void RegistersManager::registerNamesHandler(const ResultRecord& record)
{
    Q_ASSERT(!m_registerController);

    const auto registerNames = parseRegisterNames(record);
    switch (determineArchitecture(registerNames)) {
    case x86:
        qCDebug(DEBUGGERCOMMON) << "Found x86 architecture";
        setController(new RegisterController_x86(m_debugSession));
        break;
    case x86_64:
        qCDebug(DEBUGGERCOMMON) << "Found x86_64 architecture";
        setController(new RegisterController_x86_64(m_debugSession));
        break;
    case arm:
        qCDebug(DEBUGGERCOMMON) << "Found Arm architecture";
        setController(new RegisterController_Arm(m_debugSession));
        break;
    case other:
        qCWarning(DEBUGGERCOMMON) << "Unsupported architecture. Registers won't be available.";
        setController(nullptr);
        break;
    }

    if (m_registerController) {
        updateRegisters();
    }
}

void RegistersManager::setSession(MIDebugSession* debugSession)
{
    qCDebug(DEBUGGERCOMMON) << "Change session " << debugSession;
    if (m_debugSession) {
        qCDebug(DEBUGGERCOMMON) << "Will reparse arch";
        setController(nullptr);
    }
    Q_ASSERT(!m_registerController);
    m_debugSession = debugSession;
}

void RegistersManager::updateRegisters()
{
    if (!m_debugSession || m_debugSession->debuggerStateIsOn(s_dbgNotStarted | s_shuttingDown)) {
        return;
    }

    qCDebug(DEBUGGERCOMMON) << "Updating registers";
    if (m_registerController) {
        m_registersView->updateRegisters();
    } else {
        qCDebug(DEBUGGERCOMMON) << "No registerController, yet?";
        m_debugSession->addCommand(DataListRegisterNames, {}, this, &RegistersManager::registerNamesHandler);
    }
}

void RegistersManager::setController(IRegisterController* c)
{
    m_registerController.reset(c);
    m_modelsManager->setController(c);

    m_registersView->enable(c ? true : false);
}

#include "moc_registersmanager.cpp"
