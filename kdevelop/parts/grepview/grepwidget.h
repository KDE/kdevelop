/***************************************************************************
                             grepwidget.h
                             -------------------                                         
    copyright            : (C) 1999 by Bernd Gehrmann
    email                : bernd@physik.hu-berlin.de
 ***************************************************************************/

/***************************************************************************
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

class GrepDialog;
class CProject;
class GrepView;


class GrepWidget : public ProcessView
{
    Q_OBJECT
public:
    GrepWidget(GrepView *view);
    ~GrepWidget();

public slots:
    void showDialog();
    void showDialogWithPattern(QString pattern);

    // Component notifications:
    //    virtual void createConfigWidget(CustomizeDialog *parent);
    virtual void projectOpened(CProject *prj);

private slots:
    void searchActivated();
    void lineHighlighted(int line);

    
private:
    virtual void insertStdoutLine(const QString &line);

    GrepDialog *grepdlg;
    GrepView *m_view;
};

#endif
