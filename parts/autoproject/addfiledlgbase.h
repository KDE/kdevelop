/****************************************************************************
** Form interface generated from reading ui file './addfiledlgbase.ui'
**
** Created: Mon Apr 22 00:08:58 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef ADDFILEDLGBASE_H
#define ADDFILEDLGBASE_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class KLineEdit;
class KSqueezedTextLabel;
class QCheckBox;
class QGroupBox;
class QLabel;
class QPushButton;

class AddFileDlgBase : public QDialog
{ 
    Q_OBJECT

public:
    AddFileDlgBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~AddFileDlgBase();

    QGroupBox* targetBox;
    KSqueezedTextLabel* directoryLabel;
    QLabel* targetLabel;
    QLabel* directoryStaticLabel;
    QLabel* targetStaticLabel;
    QGroupBox* fileGroupBox;
    KLineEdit* fileEdit;
    QLabel* fileStaticLabel;
    QCheckBox* templateCheckBox;
    QPushButton* createButton;
    QPushButton* cancelButton;


protected:
    QVBoxLayout* AddFileDlgBaseLayout;
    QHBoxLayout* targetBoxLayout;
    QGridLayout* targetLayout;
    QHBoxLayout* buttonLayout;
};

#endif // ADDFILEDLGBASE_H
