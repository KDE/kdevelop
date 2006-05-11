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
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <kcombobox.h>


class KConfig;
class KUrlRequester;
class QPushButton;
class GrepViewPart;

class GrepDialog : public QDialog
{
    Q_OBJECT

public:
    GrepDialog( GrepViewPart * part, QWidget *parent=0, const char *name=0 );
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
	bool ignoreSCMDirsFlag() const
		{ return ignore_scm_box->isChecked(); }
	bool caseSensitiveFlag() const
		{ return case_sens_box->isChecked(); }
	bool keepOutputFlag() const
		{ return keep_output_box->isChecked(); }

	void show();

signals:
    void searchClicked();

private slots:
    void templateActivated(int index);
    void slotSearchClicked();
    void slotPatternChanged( const QString &);
	void slotSynchDirectory();

private:
    QLineEdit *template_edit;
    QComboBox *pattern_combo, *files_combo;
    KComboBox * dir_combo;
    KUrlRequester * url_requester;

    QCheckBox *recursive_box;
    QCheckBox *ignore_scm_box;
    QCheckBox *case_sens_box;
    QCheckBox *keep_output_box;
    KConfig* config;
    QPushButton *search_button;
	QPushButton *synch_button;
	GrepViewPart * m_part;
};


#endif





