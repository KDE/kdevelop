/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "testframestackmodel.h"

using namespace KDevelop;

TestFrameStackModel::TestFrameStackModel(IDebugSession* parent)
    : FrameStackModel(parent)
{
}

void TestFrameStackModel::fetchThreads()
{
}

void TestFrameStackModel::fetchFrames(int threadNumber, int from, int to)
{
    Q_UNUSED(threadNumber);
    Q_UNUSED(from);
    Q_UNUSED(to);
}

#include "moc_testframestackmodel.cpp"
