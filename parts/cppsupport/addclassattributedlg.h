/***************************************************************************
               cclassaddattributedlg.h  -  description
                          -------------------

    begin                : Fri Mar 19 1999

    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@cenacle.se

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _ADDCLASSATTRIBUTEDLG_H_
#define _ADDCLASSATTRIBUTEDLG_H_

#include <qdialog.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qmultilinedit.h>
#include <qlayout.h> 
#include <klineedit.h>
#include "parsedattribute.h"

class ClassStore;
class KCompletion;
/**
 * Dialog to create a new attibute for a class.
 * @author Jonas Nordin
 */
class AddClassAttributeDialog : public QDialog
{
    Q_OBJECT

public: // Constructor & Destructor

    AddClassAttributeDialog( ClassStore *store, ClassStore *libstore, QWidget *parent=0, const char *name=0 );
    ~AddClassAttributeDialog();
public: // Public queries

    ParsedAttribute *asSystemObj();

public: // Public widgets
    
    ///////////////////////////////
    // Layouts
    ///////////////////////////////
    
    /** Main layout for the dialog. */
    QVBoxLayout topLayout;
    
    /** Layout for function definition. */
    QGridLayout varLayout;
    
    /** Layout for choosing access. */
    QGridLayout accessLayout;
    
    /** Layout for choosing modifier. */
    QGridLayout modifierLayout;
    
    /** Layout for the ok/cancel buttons. */
    QHBoxLayout buttonLayout;
    
    ///////////////////////////////
    // Groups
    ///////////////////////////////
    
    QButtonGroup modifierGrp;
    QButtonGroup varGrp;
    QButtonGroup accessGrp;
    
    QLabel typeLbl;
    KLineEdit typeEdit;
    QLabel nameLbl;
    QLineEdit nameEdit;
    QLabel docLbl;
    QMultiLineEdit docEdit;
    
    QRadioButton publicRb;
    QRadioButton protectedRb;
    QRadioButton privateRb;
    
    QCheckBox staticCb;
    QCheckBox constCb;
    
    QPushButton okBtn;
    QPushButton cancelBtn;
    
private: // Private methods

    void setWidgetValues();
    void setCallbacks();
    void setStdCompletion();
    void setCompletion(ClassStore *store);
    
    virtual void accept();
    
private:
    KCompletion * comp;
};

#endif

