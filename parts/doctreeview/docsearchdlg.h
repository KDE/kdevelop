/***************************************************************************
 *   Copyright (C) 1999-2001 by Matthias Hoelzer-Kluepfel                  *
 *   hoelzer@kde.org                                                       *
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DOCSEARCHDLG_H_
#define _DOCSEARCHDLG_H_

#include <qdialog.h>
#include <qlineedit.h>

class QComboBox;
class QLineEdit;
class KProcess;


class DocSearchDialog : public QDialog
{
    Q_OBJECT

public:
    DocSearchDialog( QWidget *parent=0, const char *name=0 );
    ~DocSearchDialog();

    void setSearchTerm(const QString &str)
    { searchterm_edit->setText(str); }
    bool performSearch();
    
private slots:
    void htsearchStdout(KProcess *, char *buffer, int len);
    void htsearchExited(KProcess *);

private:
    virtual void accept();
    
    QLineEdit *searchterm_edit;
    QComboBox *method_combo;
    QComboBox *sort_combo;
    
    QString searchResult;
};

#endif
