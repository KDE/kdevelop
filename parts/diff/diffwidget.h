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

#ifndef _DIFFWIDGET_H_
#define _DIFFWIDGET_H_

#include <qwidget.h>

#include <kurl.h>

class QTextEdit;
class KTempFile;

namespace KIO {
  class Job;
}

namespace KParts {
  class ReadOnlyPart;
}

class DiffWidget : public QWidget
{
    Q_OBJECT

public:
    DiffWidget( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    virtual ~DiffWidget();

public slots:
    /** The URL has to point to a diff file */
    void openURL( const KURL& url );
    /** Pass a diff file in here */
    void setDiff( const QString& diff );
    /** clears the difference viewer */
    void slotClear();
    /** shows a status message */
    void showMessage( const QString& message );

private slots:
    /** appends a piece of "diff" */
    void slotAppend( const QString& str );
    /** overloaded for convenience */
    void slotAppend( KIO::Job*, const QByteArray& ba );
    /** call this when the whole "diff" has been sent.
     *  Don't call slotAppend afterwards!
     */
    void slotFinished();

private:
    /** sets komparePart to 0 if kompare part is not installed */
    void loadKomparePart( QWidget* parent );
    void setKompareVisible( bool visible );

private:
    QTextEdit* te;
    KIO::Job* job;
    KParts::ReadOnlyPart* komparePart;
    KTempFile* tempFile;
};

#endif
