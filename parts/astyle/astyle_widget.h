#ifndef __ASTYLE_WIDGET_H__
#define __ASTYLE_WIDGET_H__

#include "astyleconfig.h"

class AStylePart;
class KDevPart;


class AStyleWidget : public AStyleConfig
{
  Q_OBJECT

public:

  AStyleWidget( AStylePart * part, bool global, QWidget *parent=0, const char *name=0 );
  ~AStyleWidget();


public slots:

  void accept();


private slots:

  void styleChanged();


private:

	AStylePart * m_part;
	bool isGlobal;
};


#endif
