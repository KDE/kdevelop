/***************************************************************************
*   Copyright 1999-2001 by Bernd Gehrmann and the KDevelop Team           *
*   bernd@kdevelop.org                                                    *
*   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
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
#include <qcombobox.h>
#include <qcheckbox.h>
#include <kcombobox.h>
#include <klineedit.h>


class KConfig;
class KUrlRequester;
class QPushButton;
class GrepViewPart;
class KLineEdit;

class GrepDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GrepDialog( GrepViewPart * part, QWidget *parent=0 );
    ~GrepDialog();

    void setPattern(const QString &pattern)
    { pattern_combo->setEditText(pattern); }
    void setDirectory(const QString &dir)
    { dir_combo->setEditText(dir); }
    void setEnableProjectBox(bool enable);

    QString patternString() const
    { return pattern_combo->currentText(); }
    QString templateString() const
    { return template_edit->text(); }
    QString filesString() const
    { return files_combo->currentText(); }
    QString excludeString() const
    { return exclude_combo->currentText(); }
    QString directoryString() const
    { return dir_combo->currentText(); }

    bool useProjectFilesFlag() const
        { return use_project_box->isChecked(); }
    bool regexpFlag() const
        { return regexp_box->isChecked(); }
    bool recursiveFlag() const
        { return recursive_box->isChecked(); }
    bool noFindErrorsFlag() const
        { return no_find_err_box->isChecked(); }
    bool caseSensitiveFlag() const
        { return case_sens_box->isChecked(); }

    void show();
    void hide();

Q_SIGNALS:
    void searchClicked();

private Q_SLOTS:
    void templateActivated(int index);
    void slotSearchClicked();
    void slotPatternChanged( const QString &);
    void slotSynchDirectory();

private:
    KLineEdit *template_edit;
    KComboBox *pattern_combo, *files_combo, *exclude_combo;
    KComboBox * dir_combo;
    KUrlRequester * url_requester;

    QCheckBox *regexp_box;
    QCheckBox *recursive_box;
    QCheckBox *use_project_box;
    QCheckBox *no_find_err_box;
    QCheckBox *case_sens_box;
    KConfig* config;
    QPushButton *search_button;
    QPushButton *synch_button;
    GrepViewPart * m_part;
};


#endif





