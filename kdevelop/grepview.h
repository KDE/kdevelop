/***************************************************************************
                             grepview.h
                             -------------------                                         
    copyright            : (C) 1999 by The KDevelop Team
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#ifndef GREPVIEW_H
#define GREPVIEW_H

#include "processview.h"

class GrepDialog;
class CProject;


class GrepView : public ProcessView
{
    Q_OBJECT
public:
    GrepView(QWidget *parent, const char *name=0);
    ~GrepView();

    void showDialog();
    void showDialogWithPattern(QString pattern);

    // Component notifications:
    virtual void compilationAborted();
    virtual void projectOpened(CProject *prj);

signals:
    void itemSelected(const QString &filename, int linenumber);
    
protected slots:
    void searchActivated();
    void lineHighlighted(int line);

protected:
    virtual void insert(const QString &line);
    
private:
    GrepDialog *grepdlg;
};

#endif
