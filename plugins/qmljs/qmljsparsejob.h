/*
    SPDX-FileCopyrightText: 2012 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QMLJSPARSEJOB_H
#define QMLJSPARSEJOB_H

#include <language/backgroundparser/parsejob.h>

class QmlJsParseJob : public KDevelop::ParseJob
{
    Q_OBJECT
public:
    QmlJsParseJob(const KDevelop::IndexedString& url,
                  KDevelop::ILanguageSupport* languageSupport);

protected:
    void run(ThreadWeaver::JobPointer pointer, ThreadWeaver::Thread* thread) override;
};

#endif // PARSEQMLJSJOB_H
