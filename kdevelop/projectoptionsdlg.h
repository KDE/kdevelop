/***************************************************************************
 *   Copyright (C) 2000 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _PROJECTOPTIONSDLG_H_
#define _PROJECTOPTIONSDLG_H_

#include <kdialogbase.h>
#include <klineedit.h>

class KDevCompilerOptions;
class ServiceComboBox;


class GeneralPage : public QWidget
{
    Q_OBJECT
    
public:
    GeneralPage( QWidget *parent=0, const char *name=0 );
    ~GeneralPage();
};


class ProjectOptionsDialog : public KDialogBase
{
    Q_OBJECT
    
public:
    ProjectOptionsDialog( QWidget *parent=0, const char *name=0 );
    ~ProjectOptionsDialog();

protected:
    void accept();
    
private:
    GeneralPage *generalpage;
    CompilerPage *compilerpage;
};

#endif
