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

#ifndef _DIFFDLG_H_
#define _DIFFDLG_H_

#include <qptrlist.h>

#include <kdialogbase.h>
#include <kurl.h>

class QTextEdit;
class KTempFile;

namespace KIO {
  class Job;
}

namespace KParts {
  class ReadOnlyPart;
}

class DiffDlg : public KDialogBase
{
    Q_OBJECT

public:
    DiffDlg( QWidget *parent = 0, const char *name = 0 );
    virtual ~DiffDlg();

public slots:
    /** The URL has to point to a diff file */
    void openURL( const KURL& url );
    /** Pass a diff file in here */
    void setDiff( const QString& diff );

private slots:
    /** clears the difference viewer */
    void slotClear();
    /** appends a piece of "diff" */
    void slotAppend( const QString& str );
    /** overloaded for convenience */
    void slotAppend( KIO::Job*, const QByteArray& ba );
    /** call this when the whole "diff" has been sent.
     *  Don't call slotAppend afterwards!
     */
    void slotFinished();

private:
    /** returns false if kompare part is not installed */
    void loadKomparePart( QWidget* parent );

private:
    QTextEdit* te;
    KIO::Job* job;
    KParts::ReadOnlyPart* komparePart;
    QPtrList<KTempFile> fileCleanupHandler;

};

#endif
