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

#ifndef KDEVPLATFORM_TESTPARSEJOB_H
#define KDEVPLATFORM_TESTPARSEJOB_H

#include "language/backgroundparser/parsejob.h"

using namespace KDevelop;

class TestParseJob : public KDevelop::ParseJob
{
Q_OBJECT
public:
    TestParseJob(const IndexedString& url, ILanguageSupport* languageSupport);
    virtual void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread) override;
    virtual ControlFlowGraph* controlFlowGraph() override;
    virtual DataAccessRepository* dataAccessInformation() override;

    int duration_ms;
};

#endif
