/***************************************************************************
 *   Copyright (C) 2001 by Harald Fernengel                                *
 *   harry@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DIFFPART_H_
#define _DIFFPART_H_

#include <qpointer.h>

#include "kdevplugin.h"
#include "kdevdifffrontend.h"

class KProcess;
class DiffWidget;
class QByteArray;

class DiffPart : public KDevDiffFrontend
{
    Q_OBJECT

public:
    DiffPart(QObject *parent, const char *name, const QStringList &);
    virtual ~DiffPart();

    void openURL( const KUrl& url );
    void showDiff( const QString& diff );
    void showDiff( const KUrl &, const KUrl & ) { /** @todo */ }

public slots:
    void slotExecDiff();

private slots:
    void contextMenu( QMenu* popup, const Context* context );
    void localDiff();
    void processExited( KProcess* p );
    void receivedStdout( KProcess* p, char* buf, int buflen );
    void receivedStderr( KProcess* p, char* buf, int buflen );
    void wroteStdin( KProcess* p );

private:
    QPointer<DiffWidget> diffWidget;
    KUrl popupFile;
    KProcess* proc;
    QByteArray buffer;
    QString resultBuffer;
    QString resultErr;
};

#endif
