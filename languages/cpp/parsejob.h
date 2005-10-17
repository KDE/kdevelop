/*
 * KDevelop C++ Parse Job
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

#ifndef KDEVPARSEJOB_H
#define KDEVPARSEJOB_H

#include <Job.h>
#include <kurl.h>

class Parser;
class TranslationUnitAST;

class ParseJob : public ThreadWeaver::Job
{
    Q_OBJECT
public:
    ParseJob( const KURL &url, Parser *parser, QObject* parent );
    virtual ~ParseJob();

    void setContents( const char *contents )
    {
        m_contents = contents;
    }
    void setSize( std::size_t size )
    {
        m_size = size;
    }

    KURL document() const;
    TranslationUnitAST *translationUnit() const;

protected:
    virtual void run();

private:
    KURL m_document;
    Parser *m_parser;
    const char *m_contents;
    std::size_t m_size;
    TranslationUnitAST *m_translationUnit;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
