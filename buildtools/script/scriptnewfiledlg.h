/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _SCRIPTNEWFILEDLG_H_
#define _SCRIPTNEWFILEDLG_H_

#include <qdialog.h>

class QCheckBox;
class KLineEdit;
class ScriptProjectPart;


class ScriptNewFileDialog : public QDialog
{
    Q_OBJECT

public:
    ScriptNewFileDialog( ScriptProjectPart *part, QWidget *parent=0, const char *name=0 );
    ~ScriptNewFileDialog();
    
protected:
    virtual void accept();

private:
    QCheckBox *usetemplate_box;
    KLineEdit *filename_edit;
    ScriptProjectPart *m_part;
};

#endif
