/*
    SPDX-FileCopyrightText: 2020 Christoph Roick <chrisito@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
