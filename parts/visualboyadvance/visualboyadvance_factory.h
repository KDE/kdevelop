#ifndef __VISUALBOYADVANCE_FACTORY_H__
#define __VISUALBOYADVANCE_FACTORY_H__


#include <kdevfactory.h>

namespace VisualBoyAdvance {  
  class VisualBoyAdvanceFactory : public KDevFactory
    {
      Q_OBJECT
	
	public:
      
      VisualBoyAdvanceFactory(QObject *parent=0, const char *name=0);
      ~VisualBoyAdvanceFactory();
      
      virtual KDevPart *createPartObject(KDevApi *api, QObject *parent, const QStringList &args);
      
      static KInstance *instance();
      
      
    private:
      
      static KInstance *s_instance;
      
    };
}


#endif
