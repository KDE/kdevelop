

#include "removesubprojectdialog.h"

RemoveSubprojectDialog::RemoveSubprojectDialog(QWidget* parent, const char* name, bool modal, WFlags fl)
    : RemoveSubprojectDlgBase(parent,name, modal,fl)
{
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


#include "removesubprojectdialog.moc"

