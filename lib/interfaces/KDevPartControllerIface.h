#ifndef _KDEVPARTCONTROLLERIFACE_H_
#define _KDEVPARTCONTROLLERIFACE_H_

#include <dcopobject.h>
#include <dcopref.h>


class KDevPartController;

class KDevPartControllerIface : public QObject, public DCOPObject
{
  Q_OBJECT
  K_DCOP
    
public:
    
  KDevPartControllerIface(KDevPartController *pc);
  ~KDevPartControllerIface();


k_dcop:

  void editDocument(const QString &url, int lineNum);
  void showDocument(const QString &url, int lineNum);

  void saveAllFiles();
  void revertAllFiles();


private slots:
	
  void forwardLoadedFile(const QString &fileName);
  void forwardSavedFile(const QString &fileName);
    

private:
  
  KDevPartController *m_controller;
    
};


#endif
