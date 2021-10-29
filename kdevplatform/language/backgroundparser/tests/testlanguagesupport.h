/*
    SPDX-FileCopyrightText: 2012 Sven Brauch <svenbrauch@googlemail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TESTLANGUAGESUPPORT_H
#define KDEVPLATFORM_TESTLANGUAGESUPPORT_H

#include "language/interfaces/ilanguagesupport.h"

#include <QObject>

using namespace KDevelop;

class TestLanguageSupport
    : public QObject
    , public KDevelop::ILanguageSupport
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::ILanguageSupport)

public:
    using QObject::QObject;

    KDevelop::ParseJob* createParseJob(const IndexedString& url) override;
    QString name() const override;

Q_SIGNALS:
    void aboutToCreateParseJob(const IndexedString& url, KDevelop::ParseJob** job);
    void parseJobCreated(KDevelop::ParseJob* job);
};

#endif
