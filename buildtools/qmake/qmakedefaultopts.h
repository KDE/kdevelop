/***************************************************************************
*   Copyright (C) 2006 by Andreas Pakulat                                 *
*   apaku@gmx.de                                                          *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QMAKEDEFAULTOPTS_H
#define QMAKEDEFAULTOPTS_H

#include <qmap.h>
#include <qstringlist.h>
#include <qobject.h>

class KTempFile;
class KProcess;

class QMakeDefaultOpts : public QObject
{
    Q_OBJECT
public:
    QMakeDefaultOpts( QObject* parent = 0, const char* name = 0 );

    ~QMakeDefaultOpts();

    void readVariables( const QString& qtdir, const QString& projdir );

    const QStringList variableValues( const QString& ) const;
    const QStringList& variables() const;

signals:
    void variablesRead();

private slots:
    void slotReadStderr( KProcess*, char*, int );
    void slotFinished( KProcess* );

private:
    QMap<QString, QStringList> m_variables;
    KTempFile* makefile;
    KTempFile* qmakefile;
    KProcess* proc;
    QStringList m_keys;
};

#endif


// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
