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

#include <qguardedptr.h>
#include <qcstring.h>

#include "kdevplugin.h"
#include "kdevdifffrontend.h"

class QPopupMenu;
class Context;
class KProcess;
class DiffWidget;
class QCString;

class DiffPart : public KDevDiffFrontend
{
    Q_OBJECT

public:
    DiffPart( QObject *parent, const char *name, const QStringList & );
    virtual ~DiffPart();

    void openURL( const KURL& url );
    void showDiff( const QString& diff );
    void showDiff( const KURL &, const KURL & ) { /** @todo */ }

public slots:
    void slotExecDiff();

private slots:
    void contextMenu( QPopupMenu* popup, const Context* context );
    void localDiff();
    void processExited( KProcess* p );
    void receivedStdout( KProcess* p, char* buf, int buflen );
    void receivedStderr( KProcess* p, char* buf, int buflen );
    void wroteStdin( KProcess* p );

private:
    QGuardedPtr<DiffWidget> diffWidget;
    KURL popupFile;
    KProcess* proc;
    QCString buffer;
    QString resultBuffer;
    QString resultErr;
};

#endif
