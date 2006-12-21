
#ifndef $NEWFILENAMEUC$_H
#define $NEWFILENAMEUC$_H

#include <$QTBASECLASS$>
#include "ui_$BASEFILENAME$.h"

class $NEWCLASS$ : public $QTBASECLASS$, private Ui::$BASECLASS$
{
  Q_OBJECT

public:
  $NEWCLASS$(QWidget* parent = 0, Qt::WFlags fl = 0 );
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
