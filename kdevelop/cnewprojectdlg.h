/***************************************************************************
                          cnewprojectdlg.h  -  description
                             -------------------
    begin                : Tue Feb 9 1999
    copyright            : (C) 1999 by Steen Rabol, Ralf Nolden
    email                : rabol@get2net.dk, Ralf.Nolden@post.rwth-aachen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CNEWPROJECTDLG_H
#define CNEWPROJECTDLG_H

#include <qdialog.h>
#include <qbttngrp.h>
#include <qlabel.h>
#include <qlined.h>
#include <qpushbt.h>
#include <qradiobt.h>

#include <kapp.h>

#include "cproject.h"

class CNewProjectDlg : public QDialog{
    Q_OBJECT

public:

    CNewProjectDlg( QWidget* parent = NULL, const char* name = NULL );
    ~CNewProjectDlg();

public slots:
    QString getNewProjectFile();
    bool generatedNewProject();
protected slots:
    void slotNameChanged();
    void slotDirSelect();
    void slotOK();
protected:
    bool createdProject();
    bool createdKDE();
    bool createdKOM();
    bool createdQt();
    bool createdCplus();
    bool createdShared();
    bool createdStatic();


    CProject* newPrj;

    QButtonGroup* m_pTypeGroup;
    QButtonGroup* m_pInfoGroup;
    QLabel* m_pNameLabel;
    QLineEdit* m_pNameEdit;
    QLabel* m_pVersionLabel;
    QLineEdit* m_pVersionEdit;
    QLabel* m_pDirLabel;
    QLineEdit* m_pDirEdit;
    QPushButton* m_pDirSelect;
    QLabel* m_pAuthorLabel;
    QLineEdit* m_pAuthorEdit;
    QLabel* m_pEmailLabel;
    QLineEdit* m_pEmailEdit;
    QRadioButton* m_pTypeKDE;
    QRadioButton* m_pTypeKOM;
    QRadioButton* m_pTypeQt;
    QRadioButton* m_pTypeCplus;
    QRadioButton* m_pTypeShared;
    QRadioButton* m_pTypeStatic;
    QPushButton* m_pOK;
    QPushButton* m_pCancel;

private:
    QString genProjectFile;
    QString projectDir;
    bool success;
};
#endif // CNEWPROJECTDLG_H












