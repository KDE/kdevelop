/*
    SPDX-FileCopyrightText: 2014 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CMAKEPARSEJOB_H
#define CMAKEPARSEJOB_H

#include <language/backgroundparser/parsejob.h>

class CMakeParseJob : public KDevelop::ParseJob
{
    Q_OBJECT
    public:
        explicit CMakeParseJob(const KDevelop::IndexedString& url, KDevelop::ILanguageSupport* languageSupport);

        void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread) override;
};

#endif // CMAKEPARSEJOB_H
