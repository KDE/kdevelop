/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _GREPVIEWWIDGET_H_
#define _GREPVIEWWIDGET_H_

#include "processwidget.h"

class GrepDialog;
class GrepViewPart;
class KDevProject;


class GrepViewWidget : public ProcessWidget
{
    Q_OBJECT

public:
    GrepViewWidget(GrepViewPart *part);
    ~GrepViewWidget();

    void projectChanged(KDevProject *project);

public slots:
    void showDialog();
    void showDialogWithPattern(QString pattern);

private slots:
    void searchActivated();
    void slotExecuted(QListBoxItem *item);
    void popupMenu(QListBoxItem *, const QPoint &p);

private:
    virtual void childFinished(bool normal, int status);
    virtual void insertStdoutLine(const QString &line);

    GrepDialog *grepdlg;
    GrepViewPart *m_part;
    int m_matchCount;
    QString _lastfilename;
};

#endif
