/*
* KDevelop C++ Preprocess Job
*
* Copyright (c) 2005 Adam Treat <treat@kde.org>
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

#ifndef KDEVPREPROCESSJOB_H
#define KDEVPREPROCESSJOB_H

#include <Job.h>
#include <kurl.h>

class Preprocessor;

class PreprocessJob : public ThreadWeaver::Job
{
    Q_OBJECT
public:
    PreprocessJob( const KURL &url, Preprocessor *preprocessor,
                   QObject* parent );
    virtual ~PreprocessJob();

protected:
    virtual void run();

private:
    KURL m_document;
    Preprocessor *m_preprocessor;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
