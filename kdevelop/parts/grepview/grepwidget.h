/***************************************************************************
 *   Copyright (C) 1999, 2000 by Bernd Gehrmann                            *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _GREPWIDGET_H_
#define _GREPWIDGET_H_

#include "processview.h"

class ProjectSpace;
class KDevEditorManager;
class GrepDialog;
class GrepView;


class GrepWidget : public ProcessView
{
    Q_OBJECT
    
public:
    GrepWidget(GrepView *part);
    ~GrepWidget();

    void setProjectSpace(ProjectSpace *pProjectSpace);

public slots:
    void showDialog();
    void showDialogWithPattern(QString pattern);

private slots:
    void searchActivated();
    void lineHighlighted(int line);

private:
    virtual void childFinished(bool normal, int status);
    virtual void insertStdoutLine(const QString &line);

    GrepDialog *grepdlg;
    GrepView *m_part;
};

#endif
