/*
    SPDX-FileCopyrightText: 2013 Vlas Puhov <vlas.puhov@mail.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REGISTERSMANAGER_H
#define REGISTERSMANAGER_H

#include <QObject>
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

class RegistersManager : public QObject
{
    Q_OBJECT

public:
    explicit RegistersManager(QWidget* parent);

public Q_SLOTS:
    void setSession(MIDebugSession* debugSession);
    ///Updates all registers.
    void updateRegisters();

private:
    void registerNamesHandler(const MI::ResultRecord& record);
    void setController(IRegisterController* c);

    ModelsManager* const m_modelsManager;
    RegistersView* const m_registersView;

    MIDebugSession* m_debugSession = nullptr;
    QScopedPointer<IRegisterController> m_registerController;
};

} // end of namespace KDevMI
#endif // REGISTERSMANAGER_H
