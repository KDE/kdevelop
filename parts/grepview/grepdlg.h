/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann and the KDevelop Team       *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _GREPDLG_H_
#define _GREPDLG_H_

#define IGNORE_SCM_DIRS

#include <qdialog.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>

class KConfig;

class GrepDialog : public QDialog
{
    Q_OBJECT

public:
    GrepDialog(QWidget *parent=0, const char *name=0);
    ~GrepDialog();
    
    void setPattern(const QString &pattern)
	{ pattern_combo->setEditText(pattern); }
    void setDirectory(const QString &dir)
	{ dir_combo->setEditText(dir); }

    QString patternString() const
	{ return pattern_combo->currentText(); }
    QString templateString() const
	{ return template_edit->text(); }
    QString filesString() const
	{ return files_combo->currentText(); }
    QString directoryString() const
	{ return dir_combo->currentText(); }
    bool recursiveFlag() const
	{ return recursive_box->isChecked(); }
#ifdef IGNORE_SCM_DIRS
    bool ignoreSCMDirsFlag() const
        { return ignore_scm_box->isChecked(); }
#endif

signals:
    void searchClicked();

private slots:
    void dirButtonClicked();
    void templateActivated(int index);
    void slotSearchClicked();

private:
    QLineEdit *template_edit;
    QComboBox *dir_combo, *pattern_combo, *files_combo;
    QCheckBox *recursive_box;
#ifdef IGNORE_SCM_DIRS
    QCheckBox *ignore_scm_box;
#endif
    KConfig* config;
};


#endif





