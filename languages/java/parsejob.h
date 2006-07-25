/*
* This file is part of KDevelop
*
* Copyright (c) 2006 Adam Treat <treat@kde.org>
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

#ifndef PARSEJOB_H
#define PARSEJOB_H

#include <kurl.h>
#include <kdevparsejob.h>

#include "parser/java.h"
#include "parser/decoder.h"

using namespace java;

class KDevCodeModel;

class ParseJob : public KDevParseJob
{
    Q_OBJECT
public:
    ParseJob( const KUrl &url,
              QObject* parent,
              parser::memory_pool_type *memoryPool );

    ParseJob( KDevDocument* document,
              KTextEditor::SmartRange* highlight,
              QObject* parent,
              parser::memory_pool_type *memoryPool );

    virtual ~ParseJob();

    virtual KDevAST *AST() const;
    virtual KDevCodeModel *codeModel() const;

protected:
    virtual void run();

private:
    parser::memory_pool_type *m_memoryPool;
    compilation_unit_ast *m_AST;
    KDevCodeModel *m_model;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
