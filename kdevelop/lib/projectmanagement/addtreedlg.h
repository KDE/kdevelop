#ifndef ADDTREEDLG_H
#define ADDTREEDLG_H

#include "addtreedlgbase.h"
#include "qstringlist.h"

class AddTreeDlg : public AddTreeDlgBase { 
    Q_OBJECT
      
public:
    AddTreeDlg( QWidget* parent = 0, const char* name = 0, QString strartDir="/",QStringList filters ="");
    ~AddTreeDlg();
    bool recursive();
    QString directory();
    QString filter();
};

#endif // ADDTREEDLG_H
