/***************************************************************************
                          griddlg  -  description
                             -------------------                                         
    begin                : Wed Mar 17 1999                                           
    copyright            : (C) 1999 by Ralf Nolden
    email                : Ralf.Nolden@post.rwth-aachen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#include <qdialog.h>
#include <qcheckbox.h>

class QSpinBox;
class QLabel;
class QPushButton;

class GridDlg : public QDialog
{
    Q_OBJECT
	public:
    GridDlg(QWidget* parent=0,const char * name=0, int x=0, int y=0 /*, bool modal=FALSE, WFlags f=0*/ );
    ~GridDlg();
    
    int getGridX();
    int getGridY();
    
    public slots:
	void slotVHSameClicked();
    
 protected:
    QSpinBox *sbVGrid, *sbHGrid;
    QLabel *lbVGrid, *lbHGrid;
    
    QCheckBox  *cbVHSame;
    
    QPushButton *pbOk, *pbCancel;
};

