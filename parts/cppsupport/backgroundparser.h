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

#ifndef BACKGROUNDPARSER_H
#define BACKGROUNDPARSER_H

#include <qthread.h>
#include <qstring.h>

class BackgroundParser: public QThread{
public:
    BackgroundParser( class CppSupportPart*, const QString& filename );
    virtual ~BackgroundParser();

    virtual void run();

private:
    class CppSupportPart* m_cppSupport;
    QString m_fileName;
};

#endif
