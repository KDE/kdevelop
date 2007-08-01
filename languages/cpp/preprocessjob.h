/*
* This file is part of KDevelop
*
* Copyright 2006 Adam Treat <treat@kde.org>
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

#ifndef PREPROCESSJOB_H
#define PREPROCESSJOB_H

class CPPParseJob;

#include <parsejob.h>
#include <ksharedptr.h>

#include "parser/rpp/preprocessor.h"

namespace Cpp {
    class EnvironmentFile;
}

class CppPreprocessEnvironment;

class PreprocessJob : public ThreadWeaver::Job, public rpp::Preprocessor
{
    Q_OBJECT
public:
    PreprocessJob(CPPParseJob* parent);

    CPPParseJob* parentJob() const;

    virtual rpp::Stream* sourceNeeded(QString& fileName, IncludeType type, int sourceLine, bool skipCurrentPath);

    virtual bool success() const;

    //Must only be called for direct parsing when the job is not queued
    virtual void run();

private:
    bool checkAbort();

    CppPreprocessEnvironment* m_currentEnvironment;
    KSharedPtr<Cpp::EnvironmentFile> m_environmentFile;
    bool m_success;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
