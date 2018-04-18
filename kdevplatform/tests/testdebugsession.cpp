/*
 * This file is part of KDevelop
 *
 * Copyright 2016  Aetf <aetf@unlimitedcodeworks.xyz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
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

void TestDebugSession::setBreakpointController(IBreakpointController *breakpointController)
{
    delete m_breakpointController;
    m_breakpointController = breakpointController;
    if (!m_breakpointController) {
        m_breakpointController = new TestBreakpointController(this);
    }
}

void TestDebugSession::setVariableController(IVariableController *variableController)
{
    delete m_variableController;
    m_variableController = variableController;
    if (!m_variableController) {
        m_variableController = new TestVariableController(this);
    }
}

void TestDebugSession::setFrameStackModel(IFrameStackModel *frameStackModel)
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
