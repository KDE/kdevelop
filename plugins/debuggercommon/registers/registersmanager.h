/*
    SPDX-FileCopyrightText: 2013 Vlas Puhov <vlas.puhov@mail.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REGISTERSMANAGER_H
#define REGISTERSMANAGER_H

#include <QObject>
#include <QStringList>
#include <QScopedPointer>

namespace KDevMI {
namespace MI
{
struct ResultRecord;
}

class MIDebugSession;

class RegistersView;
class IRegisterController;
class ModelsManager;

enum Architecture {x86, x86_64, arm, other = 100, undefined};

/** @brief Determines current CPU architecture */
class ArchitectureParser : public QObject
{
    Q_OBJECT

public:

    explicit ArchitectureParser(QObject* parent);

    ///Asynchronously determines current architecture. emits @p architectureParsed when ready.
    void determineArchitecture(MIDebugSession* debugSession);

Q_SIGNALS:
    ///Emits current CPU architecture. @sa determineArchitecture
    void architectureParsed(Architecture arch);

private:

    void registerNamesHandler(const MI::ResultRecord& r);
    void parseArchitecture();

    QStringList m_registerNames;
};

class RegistersManager : public QObject
{
    Q_OBJECT

public:
    explicit RegistersManager(QWidget* parent);

public Q_SLOTS:
    void setSession(MIDebugSession* debugSession);
    ///Updates all registers.
    void updateRegisters();
    ///@sa ArchitectureParser::determineArchitecture
    void architectureParsedSlot(const Architecture arch);

private:
    void setController(IRegisterController* c);

    ModelsManager* const m_modelsManager;
    RegistersView* const m_registersView;

    MIDebugSession* m_debugSession = nullptr;
    QScopedPointer<IRegisterController> m_registerController;

    ArchitectureParser* const m_architectureParser;
    Architecture m_currentArchitecture = undefined;
    ///True if architecture could has changed(e.g. from x86 to arm)
    bool m_needToCheckArch = false;
};

} // end of namespace KDevMI
#endif // REGISTERSMANAGER_H
