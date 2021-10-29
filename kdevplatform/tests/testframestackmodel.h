/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TESTFRAMESTACKMODEL_H
#define TESTFRAMESTACKMODEL_H

#include "testsexport.h"

#include <debugger/framestack/framestackmodel.h>

namespace KDevelop {
/**
 * Dummy frame stack model
 */
class KDEVPLATFORMTESTS_EXPORT TestFrameStackModel
    : public FrameStackModel
{
    Q_OBJECT

public:
    explicit TestFrameStackModel(IDebugSession* parent);

protected:
    void fetchThreads() override;
    void fetchFrames(int threadNumber, int from, int to) override;

private:
};
} // end of namespace KDevelop
#endif // TESTFRAMESTACKMODEL_H
