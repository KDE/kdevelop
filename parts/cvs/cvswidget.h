/***************************************************************************
 *   Copyright (C) 2003 by KDevelop Authors                                *
 *   www.kdevelop.org                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CVSWIDGET_H_
#define _CVSWIDGET_H_

#include <qwidget.h>

#include "processwidget.h"

class CvsPart;

class CvsWidget : public ProcessWidget
{
    Q_OBJECT
public:
    CvsWidget(CvsPart *part);
    ~CvsWidget();
    
public slots:
    void startCommand(const QString &dir, const QString &command);

private slots:
    void lineHighlighted(int line);

private:
    virtual void insertStdoutLine(const QString & line);
    virtual void childFinished(bool normal, int status);

    CvsPart * m_part;
    QString dir;
};

#endif

