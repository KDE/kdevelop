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

#ifndef BACKGROUNDPARSER_H
#define BACKGROUNDPARSER_H

#include <qthread.h>
#include <qstring.h>

class ProblemReporter;

class BackgroundParser: public QThread{
public:
    BackgroundParser( ProblemReporter* reporter,
                      const QString& source,
                      const QString& filename );
    virtual ~BackgroundParser();

    virtual void run();

private:
    ProblemReporter* m_reporter;
    QString m_source;
    QString m_fileName;
};

#endif
