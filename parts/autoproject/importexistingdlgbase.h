/****************************************************************************
** Form interface generated from reading ui file './importexistingdlgbase.ui'
**
** Created: Wed Apr 24 20:59:33 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef IMPORTEXISTINGDLGBASE_H
#define IMPORTEXISTINGDLGBASE_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QGroupBox;
class QPushButton;

class ImportExistingDlgBase : public QDialog
{ 
    Q_OBJECT

public:
    ImportExistingDlgBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~ImportExistingDlgBase();

    QGroupBox* sourceGroupBox;
    QGroupBox* destGroupBox;
    QPushButton* helpButton;
    QPushButton* okButton;
    QPushButton* cancelButton;


protected:
    QVBoxLayout* ImportExistingDlgBaseLayout;
    QHBoxLayout* viewLayout;
    QVBoxLayout* sourceGroupBoxLayout;
    QVBoxLayout* destGroupBoxLayout;
    QHBoxLayout* buttonLayout;
};

#endif // IMPORTEXISTINGDLGBASE_H
