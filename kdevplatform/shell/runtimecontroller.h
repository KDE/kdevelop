/*
    SPDX-FileCopyrightText: 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_RUNTIMECONTROLLER_H
#define KDEVPLATFORM_RUNTIMECONTROLLER_H

#include <interfaces/iruntimecontroller.h>
#include <interfaces/iruntime.h>

class RuntimeViewFactory;
class QActionGroup;
class QMenu;

namespace KDevelop
{

class Core;

class RuntimeController : public IRuntimeController
{
    Q_OBJECT
public:
    explicit RuntimeController(Core* core);
    ~RuntimeController() override;

    void initialize();

    void addRuntimes(KDevelop::IRuntime * runtimes) override;
    QVector<KDevelop::IRuntime *> availableRuntimes() const override;

    KDevelop::IRuntime * currentRuntime() const override;
    void setCurrentRuntime(KDevelop::IRuntime * runtime) override;

private:
    void setupActions();

private:
    QScopedPointer<QMenu> m_runtimesMenu;
    QVector<IRuntime*> m_runtimes;
    IRuntime* m_currentRuntime = nullptr;
    Core* const m_core;
    QActionGroup* const m_group;
};

}

#endif
