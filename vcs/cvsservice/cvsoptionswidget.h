/***************************************************************************
 *   Copyright (C) 2003 by KDevelop Authors                                *
 *   kdevelop-devel@kde.org                                                *
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CVSOPTIONSWIDGET_H_
#define _CVSOPTIONSWIDGET_H_

#include <qwidget.h>
#include "cvsoptionswidgetbase.h"

class QLabel;
class QLineEdit;
class QVBoxLayout;

class CvsOptionsWidget : public CvsOptionsWidgetBase
{
    Q_OBJECT
public:
    CvsOptionsWidget( QWidget *parent, const char *name=0 );
    virtual ~CvsOptionsWidget();

    bool pruneEmptyDirWhenUpdating() const;
    void setPruneEmptyDirWhenUpdating( bool b );

    bool createNewDirWhenUpdating() const;
    void setCreateNewDirWhenUpdating( bool b );

    bool recursiveWhenUpdating() const;
    void setRecursiveWhenUpdating( bool b );

    bool recursiveWhenCommittingRemoving() const;
    void setRecursiveWhenCommittingRemoving( bool b );

    unsigned int contextLines() const;
    void setContextLines( unsigned int p );

    QString diffOptions() const;
    void setDiffOptions( const QString &p );

    QString cvsRshEnvVar() const;
    void setCvsRshEnvVar( const QString &p );

    QString serverLocation() const;
    void setServerLocation( const QString &p );

public slots:
    void accept();

private:
    void readConfig();
    void storeConfig();
};

#endif
