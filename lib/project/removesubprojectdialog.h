
#ifndef REMOVESUBPROJECTDIALOG_H
#define REMOVESUBPROJECTDIALOG_H

#include "removesubprojectdlgbase.h"

class RemoveSubprojectDialog : public RemoveSubprojectDlgBase
{
  Q_OBJECT

public:
  RemoveSubprojectDialog(QString caption, QString question, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
  ~RemoveSubprojectDialog();
  /*$PUBLIC_FUNCTIONS$*/
  bool removeFromDisk();

public slots:
  /*$PUBLIC_SLOTS$*/

protected:
  /*$PROTECTED_FUNCTIONS$*/

protected slots:
  /*$PROTECTED_SLOTS$*/
  virtual void          reject();
  virtual void          accept();

};

#endif

