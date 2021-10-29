/*
    SPDX-FileCopyrightText: 2014 David Stevens <dgedstevens@gmail.com>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TESTCODECOMPLETION_H
#define TESTCODECOMPLETION_H

#include "codecompletiontestbase.h"

class TestCodeCompletion : public CodeCompletionTestBase
{
    Q_OBJECT

private Q_SLOTS:
    void testIncludePathCompletion_data();
    void testIncludePathCompletion();
    void testIncludePathCompletionLocal();

    void testClangCodeCompletion();
    void testClangCodeCompletion_data();
    void testClangCodeCompletionType();
    void testClangCodeCompletionType_data();
    void testVirtualOverride();
    void testVirtualOverride_data();
    void testOverrideExecute();
    void testOverrideExecute_data();
    void testImplement();
    void testImplement_data();
    void testImplementOtherFile();
    void testImplementAfterEdit();
    void testInvalidCompletions();
    void testInvalidCompletions_data();
    void testCompletionPriority();
    void testCompletionPriority_data();
    void testReplaceMemberAccess();
    void testReplaceMemberAccess_data();
    void testArgumentHintCompletion();
    void testArgumentHintCompletion_data();

    void testOverloadedFunctions();
    void testVariableScope();
    void testArgumentHintCompletionDefaultParameters();

    void testCompleteFunction_data();
    void testCompleteFunction();

    void testIgnoreGccBuiltins();
};

#endif // TESTCODECOMPLETION_H
