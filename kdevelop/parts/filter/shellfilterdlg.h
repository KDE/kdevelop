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

#ifndef _SHELLFILTERDLG_H_
#define _SHELLFILTERDLG_H_

#include <qdialog.h>

class QComboBox;
class QPushButton;
class KDevPlugin;
class KProcess;


class ShellFilterDialog : public QDialog
{
    Q_OBJECT

public:
    ShellFilterDialog();
    ~ShellFilterDialog();

    virtual int exec();

    void setText(const QString &str)
    { m_instr = str.local8Bit(); }
    QString text() const
    { return m_outstr; }

private slots:
    void slotStartClicked();
    void slotReceivedStdout(KProcess *, char *text, int len);
    void slotWroteStdin(KProcess *);
    void slotProcessExited(KProcess *);

private:
    QPushButton *start_button, *cancel_button;
    QComboBox *combo;
    KProcess *m_proc;
    QCString m_instr;
    QString m_outstr;
};

#endif
