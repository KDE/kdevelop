/***************************************************************************
 *   Copyright (C) 1999, 2000 by Bernd Gehrmann                            *
 *   bernd@kdevelop.org                                                    *
 *   Modified for perforce 2002 by Harald Fernengel <harry@kdevelop.org>   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _COMMITDIALOG_H_
#define _COMMITDIALOG_H_

#include <qstringlist.h>
#include <qtextedit.h>
#include <klineedit.h>
#include <klistbox.h>
#include <kdialogbase.h>

class KProcess;

class CommitDialog : public KDialogBase
{
    Q_OBJECT
public:
    CommitDialog( QWidget *parent = 0, const char *name = 0 );
    ~CommitDialog();

    QString logMessage() const { return edit->text(); }
    QString user() const { return userEdit->text(); }
    QString client() const { return clientEdit->text(); }
    QString changeList() const;
    
    void setUser( const QString& usr ) { userEdit->setText( usr ); }
    void setClient( const QString& clnt ) { clientEdit->setText( clnt ); }
    void setFiles( const QStringList& lst ); 

    /** tries to fill out user and client */
    void autoGuess();    

protected slots:
    void accept();

private slots:
    void getFilesFinished( const QString& out, const QString& err );

private:
    void setDepotFiles( const QStringList& lst );
    QTextEdit *edit;
    KLineEdit *clientEdit, *userEdit;
    KListBox  *filesBox;
};

#endif

