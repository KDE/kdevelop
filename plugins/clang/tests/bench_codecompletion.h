/*
    SPDX-FileCopyrightText: 2016 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef BENCHCODECOMPLETION_H
#define BENCHCODECOMPLETION_H

#include "codecompletiontestbase.h"

class ClangIndex;
class ClangCodeCompletionModel;

class BenchCodeCompletion : public CodeCompletionTestBase
{
    Q_OBJECT

public:
    BenchCodeCompletion();
    ~BenchCodeCompletion() override;

private Q_SLOTS:
    void benchCodeCompletion_data();
    void benchCodeCompletion();

private:
    QScopedPointer<ClangIndex> m_index;
    ClangCodeCompletionModel* m_model;
};

#endif // BENCHCODECOMPLETION_H
