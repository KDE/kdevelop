#include <qcheckbox.h>
#include <qlabel.h>
#include "removesubprojectdialog.h"

RemoveSubprojectDialog::RemoveSubprojectDialog(QString caption, QString question, QWidget* parent, const char* name, bool modal, WFlags fl)
    : RemoveSubprojectDlgBase(parent,name, modal,fl)
{
    setCaption(caption);
    removeLabel->setText(question);
}

RemoveSubprojectDialog::~RemoveSubprojectDialog()
{
}

/*$SPECIALIZATION$*/
void RemoveSubprojectDialog::reject()
{
    QDialog::reject();
}

void RemoveSubprojectDialog::accept()
{
    QDialog::accept();
}

bool RemoveSubprojectDialog::removeFromDisk( )
{
    return removeCheckBox->isChecked();
}


#include "removesubprojectdialog.moc"

