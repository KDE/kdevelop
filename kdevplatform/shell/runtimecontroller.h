/*
   Copyright 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_RUNTIMECONTROLLER_H
#define KDEVPLATFORM_RUNTIMECONTROLLER_H

#include <interfaces/iruntimecontroller.h>
#include <interfaces/iruntime.h>

class RuntimeViewFactory;
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
};

}

#endif
