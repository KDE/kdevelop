/***************************************************************************
     

    begin                : Wed Jan 5 2000
    copyright            : (C) 2000 by Sandy Meier                         
    email                : smeier@rz.uni-potsdam.de                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/
#ifndef FILECLOSEALLDLG_H
#define FILECLOSEALLDLG_H

#include <qdialog.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qstringlist.h>

class FileCloseAllDlg : public QDialog
{
    Q_OBJECT

public:

    FileCloseAllDlg(QWidget* parent =0,const char* name = 0,QStringList* files=0);
    
 protected:
    QPushButton* sellect_all_button;
    QPushButton* sellect_none_button;
    QPushButton* ok_button;
    QPushButton* cancel_button;
    QListView* files_listview;
    protected slots:
    void slotSelectAll();
    void slotSelectNone();
    void slotOK();
};
#endif 
