/****************************************************************************
** Form interface generated from reading ui file './librarydocdlgbase.ui'
**
** Created: Sam Nov 9 16:57:39 2002
**      by: The User Interface Compiler ()
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef LIBRARYDOCDLG_H
#define LIBRARYDOCDLG_H

#include "librarydocdlgbase.h"

class KLineEdit;
class KURLRequester;

class LibraryDocDlg : public LibraryDocDlgBase
{
    Q_OBJECT

public:
    LibraryDocDlg( QWidget* parent, const char* name, const QString& path, const QString& _default, const QString group);
    ~LibraryDocDlg();

private:
    QString mName;
    QString mPath;
    QString mDef;
    QString m_group;

protected slots:
    void accept();
    void slotDefault();
};

#endif // LIBRARYDOCDLG_H
