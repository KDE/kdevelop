/*
 *  Copyright (C) 2001 Sandy Meier <smeier@kdevelop.org>   
 */
 

#ifndef __KDEVPART_VISUALBOYADVANCE_H__
#define __KDEVPART_VISUALBOYADVANCE_H__
  
#include <qguardedptr.h>
#include <kdevplugin.h>
#include <kdialogbase.h>

namespace VisualBoyAdvance {  
  class VisualBoyAdvancePart : public KDevPlugin
    {
      Q_OBJECT
	
	public:
      
      VisualBoyAdvancePart(QObject *parent, const char *name, const QStringList &);
      ~VisualBoyAdvancePart();
      private slots:
	void slotExecute();
      void projectConfigWidget(KDialogBase *dlg);
      
    };
  
}
#endif
