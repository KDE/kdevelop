/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2003 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _ADDDOCITEMDLG_H_
#define _ADDDOCITEMDLG_H_

#include <qdialog.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <kurlrequester.h>
#include <kfile.h>
class QPushButton;

class AddDocItemDialog : public QDialog
{
    Q_OBJECT

public:
    enum TitleType {None, Qt, KDevelopTOC, DevHelp};

    AddDocItemDialog( KFile::Mode mode = KFile::File, QString filter = "text/html",
        TitleType checkDocTitle = None, QString title = "", QString url = "", QWidget *parent=0, const char *name=0 );
    ~AddDocItemDialog();

    QString title() const
        { return title_edit->text(); }
    QString url() const
        { return url_edit->url(); }

private slots:
    void setTitle(const QString &str);
    void setLocationChanged(const QString & _text );
private:
    QLineEdit *title_edit;
    KURLRequester *url_edit;
    KFile::Mode m_mode;
    TitleType m_type;
    QString m_filter;
    QCheckBox *title_check;
    QPushButton *m_pOk;
};

#endif
