/***************************************************************************
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _SHELLINSERTDLG_H_
#define _SHELLINSERTDLG_H_

#include <qdialog.h>

class QComboBox;
class QPushButton;
class KDevPlugin;
class KProcess;


class ShellInsertDialog : public QDialog
{
    Q_OBJECT
    
public:
    ShellInsertDialog();
    ~ShellInsertDialog();

    virtual int exec();

    QString text() const
    { return QString::fromLocal8Bit(m_str); }
    
private slots:
    void slotStartClicked();
    void slotReceivedStdout(KProcess *, char *text, int len);
    void slotProcessExited(KProcess *);
    void executeTextChanged( const QString &text);
   
private:
    QPushButton *start_button, *cancel_button;
    QComboBox *combo;
    KProcess *m_proc;
    QCString m_str;
};

#endif
