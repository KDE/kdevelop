/*
 *  Copyright (C) 2001 Sandy Meier <smeier@kdevelop.org>   
 */
 

#ifndef __KDEVPART_VISUALBOYADVANCE_H__
#define __KDEVPART_VISUALBOYADVANCE_H__
  
#include <qguardedptr.h>
#include <kdevpart.h>
#include <kdialogbase.h>

namespace VisualBoyAdvance {  
  class VisualBoyAdvancePart : public KDevPart
    {
      Q_OBJECT
	
	public:
      
      VisualBoyAdvancePart(KDevApi *api, QObject *parent=0, const char *name=0);
      ~VisualBoyAdvancePart();
      private slots:
	void slotExecute();
      void projectConfigWidget(KDialogBase *dlg);
      
    };
  
}
#endif
