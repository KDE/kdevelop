/****************************************************************************
** Form interface generated from reading ui file './addexistingdlgbase.ui'
**
** Created: Sam Dez 21 17:56:52 2002
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef ADDEXISTINGDLGBASE_H
#define ADDEXISTINGDLGBASE_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class KProgress;
class KSqueezedTextLabel;
class QGroupBox;
class QLabel;
class QPushButton;
class QSplitter;

class AddExistingDlgBase : public QDialog
{
    Q_OBJECT

public:
    AddExistingDlgBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~AddExistingDlgBase();

    QGroupBox* infoGroupBox;
    QLabel* directoryStaticLabel;
    QLabel* targetStaticLabel;
    KSqueezedTextLabel* directoryLabel;
    QLabel* targetLabel;
    QSplitter* Splitter2;
    QGroupBox* sourceGroupBox;
    QPushButton* addAllButton;
    QPushButton* addSelectedButton;
    QLabel* destStaticLabel;
    KSqueezedTextLabel* destLabel;
    QGroupBox* destGroupBox;
    QPushButton* removeAllButton;
    QPushButton* removeSelectedButton;
    KProgress* progressBar;
    QPushButton* okButton;
    QPushButton* cancelButton;

protected:
    QVBoxLayout* AddExistingDlgBaseLayout;
    QHBoxLayout* infoGroupBoxLayout;
    QVBoxLayout* infoLayout1;
    QVBoxLayout* infoLayout2;
    QHBoxLayout* Layout11;
    QVBoxLayout* sourceGroupBoxLayout;
    QVBoxLayout* Layout10;
    QHBoxLayout* Layout13;
    QVBoxLayout* Layout9;
    QHBoxLayout* Layout8;
    QVBoxLayout* destGroupBoxLayout;
    QVBoxLayout* Layout10_2;
    QHBoxLayout* layout10;

protected slots:
    virtual void languageChange();
};

#endif // ADDEXISTINGDLGBASE_H
