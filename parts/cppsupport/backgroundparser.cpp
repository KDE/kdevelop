/***************************************************************************
 *   Copyright (C) 2002 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "backgroundparser.h"
#include "cppsupportpart.h"
#include "problemreporter.h"
#include "filerepository.h"
#include "ast.h"

#include <kdebug.h>

BackgroundParser::BackgroundParser( CppSupportPart* part, const QString& filename )
    : m_cppSupport(part),
      m_fileName( filename.unicode(), filename.length() )
{
}

BackgroundParser::~BackgroundParser()
{
}

void BackgroundParser::run()
{
    QValueList<Problem> problems = m_cppSupport->fileRepository()->problems( m_fileName, true );
    
    for( QValueList<Problem>::Iterator it = problems.begin(); it != problems.end(); ++it ){
	const Problem& p = *it;
	m_cppSupport->problemReporter()->reportError( p.text(), m_fileName, p.line(), p.column() );
    }
    
    kdDebug(9007) << "FINISHED!!" << endl;
}



