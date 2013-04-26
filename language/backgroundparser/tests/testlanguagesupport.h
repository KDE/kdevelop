/*
 * This file is part of KDevelop
 *
 * Copyright 2012 by Sven Brauch <svenbrauch@googlemail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_TESTLANGUAGESUPPORT_H
#define KDEVPLATFORM_TESTLANGUAGESUPPORT_H

#include "language/interfaces/ilanguagesupport.h"

#include <QObject>

using namespace KDevelop;

class TestLanguageSupport : public QObject, public KDevelop::ILanguageSupport
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::ILanguageSupport)
public:
    virtual KDevelop::ParseJob* createParseJob(const IndexedString& url);
    virtual QString name() const;

signals:
    void parseJobCreated(KDevelop::ParseJob* job);
};

#endif
