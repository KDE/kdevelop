/* This file is part of KDevelop
 *
 * Copyright 2020 Christoph Roick <chrisito@gmx.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/**
 * Required implementations to inherit from MIDebuggerPlugin to test the DBus interface
 */

#include <KPluginFactory>

#include "../midebugger.h"
#include "../midebuggerplugin.h"
#include "../midebugsession.h"

#include <QObject>
#include <QTimer>

class TestDebugger : public KDevMI::MIDebugger
{
public:
    TestDebugger() : KDevMI::MIDebugger() {}
    bool start(KConfigGroup&, const QStringList&) override { return true; }
};

class TestDebugSession : public KDevMI::MIDebugSession
{
    Q_OBJECT
public:
    explicit TestDebugSession(KDevMI::MIDebuggerPlugin *plugin = nullptr) : KDevMI::MIDebugSession(plugin)
    {}

protected:
    KDevMI::MIDebugger *createDebugger() const override
    {
        return new TestDebugger;
    }
    void initializeDebugger() override
    {
        QTimer::singleShot(500, this, &TestDebugSession::finished);
    }
    void configInferior(KDevelop::ILaunchConfiguration *, IExecutePlugin *, const QString &) override {}
    bool execInferior(KDevelop::ILaunchConfiguration *, IExecutePlugin *, const QString &) override { return false; }
    bool loadCoreFile(KDevelop::ILaunchConfiguration *, const QString &, const QString &) override { return false; }
    KDevMI::MIBreakpointController * breakpointController() const override { return nullptr; }
    KDevelop::IFrameStackModel* frameStackModel() const override { return nullptr; }
    KDevelop::IVariableController* variableController() const override { return nullptr; }
};

class TestDebuggerPlugin : public KDevMI::MIDebuggerPlugin
{
    Q_OBJECT
public:
    TestDebuggerPlugin(QObject *parent, const QVariantList&) : MIDebuggerPlugin("testdebugger", "Test", parent) {}
    ~TestDebuggerPlugin() {}

    KDevMI::MIDebugSession *createSession() override
    {
        auto* session = new TestDebugSession(this);
        connect(session, &TestDebugSession::showMessage, this, &TestDebuggerPlugin::showStatusMessage);
        return session;
    }
    void setupToolViews() override {}
    void unloadToolViews() override {}
};

K_PLUGIN_FACTORY_WITH_JSON(TestDebuggerFactory, "testdebugger.json", registerPlugin<TestDebuggerPlugin>(); )

#include "testdebugger.moc"
