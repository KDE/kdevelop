/***************************************************************************
                             doctreeconfdlg.h
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


#ifndef DOCTREECONFDLG_H
#define DOCTREECONFDLG_H

#include <qdialog.h>


class DocTreeConfigDialog : public QDialog
{
    Q_OBJECT
	
public: 
    DocTreeConfigDialog(QWidget *parent=0, const char *name=0);
    ~DocTreeConfigDialog();

    virtual void accept();

private:
    void readConfig();
    void storeConfig();
};

#endif

