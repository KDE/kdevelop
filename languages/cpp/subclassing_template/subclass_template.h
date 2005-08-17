
#ifndef $NEWFILENAMEUC$_H
#define $NEWFILENAMEUC$_H

#include "$BASEFILENAME$.h"

class $NEWCLASS$ : public $BASECLASS$
{
  Q_OBJECT

public:
  $NEWCLASS$(QWidget* parent = 0, const char* name = 0$CAN_BE_MODAL_H$, Qt::WFlags fl = 0 );
  ~$NEWCLASS$();
  /*$PUBLIC_FUNCTIONS$*/

public slots:
  /*$PUBLIC_SLOTS$*/

protected:
  /*$PROTECTED_FUNCTIONS$*/

protected slots:
  /*$PROTECTED_SLOTS$*/

};

#endif
