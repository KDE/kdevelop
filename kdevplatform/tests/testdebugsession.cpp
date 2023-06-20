/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "testdebugsession.h"

#include "testbreakpointcontroller.h"
#include "testvariablecontroller.h"
#include "testframestackmodel.h"

using namespace KDevelop;

TestDebugSession::TestDebugSession()
{
    setBreakpointController(nullptr);
    setVariableController(nullptr);
    setFrameStackModel(nullptr);
}

TestDebugSession::~TestDebugSession()
{
    delete m_breakpointController;
    delete m_variableController;
    delete m_frameStackModel;
}

void TestDebugSession::setBreakpointController(IBreakpointController* breakpointController)
{
    delete m_breakpointController;
    m_breakpointController = breakpointController;
    if (!m_breakpointController) {
        m_breakpointController = new TestBreakpointController(this);
    }
}

void TestDebugSession::setVariableController(IVariableController* variableController)
{
    delete m_variableController;
    m_variableController = variableController;
    if (!m_variableController) {
        m_variableController = new TestVariableController(this);
    }
}

void TestDebugSession::setFrameStackModel(IFrameStackModel* frameStackModel)
{
    delete m_frameStackModel;
    m_frameStackModel = frameStackModel;
    if (!m_frameStackModel) {
        m_frameStackModel = new TestFrameStackModel(this);
    }
}

IDebugSession::DebuggerState TestDebugSession::state() const
{
    return m_sessionState;
}

bool TestDebugSession::restartAvaliable() const
{
    return true;
}

IBreakpointController* TestDebugSession::breakpointController() const
{
    return m_breakpointController;
}

IVariableController* TestDebugSession::variableController() const
{
    return m_variableController;
}

IFrameStackModel* TestDebugSession::frameStackModel() const
{
    return m_frameStackModel;
}

void TestDebugSession::restartDebugger()
{
    m_sessionState = ActiveState;
}

void TestDebugSession::stopDebugger()
{
    m_sessionState = StoppedState;
}

void TestDebugSession::killDebuggerNow()
{
    m_sessionState = StoppedState;
}

void TestDebugSession::interruptDebugger()
{
    m_sessionState = StoppedState;
}

void TestDebugSession::run()
{
    m_sessionState = ActiveState;
}

void TestDebugSession::runToCursor()
{
    m_sessionState = PausedState;
}

void TestDebugSession::jumpToCursor()
{
    m_sessionState = PausedState;
}

void TestDebugSession::stepOver()
{
    m_sessionState = PausedState;
}

void TestDebugSession::stepIntoInstruction()
{
    m_sessionState = PausedState;
}

void TestDebugSession::stepInto()
{
    m_sessionState = PausedState;
}

void TestDebugSession::stepOverInstruction()
{
    m_sessionState = PausedState;
}

void TestDebugSession::stepOut()
{
    m_sessionState = PausedState;
}

#include "moc_testdebugsession.cpp"
