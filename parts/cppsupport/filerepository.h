/***************************************************************************
 *   Copyright (C) 2002 by Roberto Raggi                                   *
 *   raggi@cli.di.unipi.it                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __filerepository_h
#define __filerepository_h

#include <qobject.h>
#include "parser.h"

namespace KParts{ class Part; }

class FileRepository: public QObject
{
    Q_OBJECT
public:
    FileRepository( class CppSupportPart* part );
    virtual ~FileRepository();
    
    class TranslationUnitAST* translationUnit( const QString& fileName, bool reparse=false );
    QValueList<Problem> problems( const QString& fileName, bool reparse=false );
    
private slots:
    void slotPartAdded( KParts::Part* );
    void slotPartRemoved( KParts::Part* );
    
private:
    class FileRepositoryData* d;
};

#endif
