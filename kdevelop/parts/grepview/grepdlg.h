/***************************************************************************
 *   Copyright (C) 1999, 2000 by Bernd Gehrmann                            *
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

#include <qdialog.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>


class GrepDialog : public QDialog
{
    Q_OBJECT

public:
    GrepDialog(QWidget *parent=0, const char *name=0);
    ~GrepDialog();
    void setPattern(const QString &pattern)
	{ pattern_edit->setText(pattern); }
    void setDirectory(const QString &dir)
	{ dir_edit->setText(dir); }

    QString patternString()
	{ return pattern_edit->text(); }
    QString templateString()
	{ return template_edit->text(); }
    QString filesString()
	{ return files_combo->currentText(); }
    QString directoryString()
	{ return dir_edit->text(); }
    bool recursiveFlag()
	{ return recursive_box->isChecked(); }

signals:
    void searchClicked();

private slots:
    void dirButtonClicked();
    void templateActivated(int index);
    void slotSearchClicked();

private:
    QLineEdit *pattern_edit, *template_edit, *dir_edit;
    QComboBox *files_combo;
    QCheckBox *recursive_box;
};


#endif





