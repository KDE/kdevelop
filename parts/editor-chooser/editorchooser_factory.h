#ifndef __EDITORCHOOSER_FACTORY_H__
#define __EDITORCHOOSER_FACTORY_H__


#include <kdevfactory.h>


class EditorChooserFactory : public KDevFactory
{
  Q_OBJECT

public:

  EditorChooserFactory(QObject *parent=0, const char *name=0);
  ~EditorChooserFactory();

  virtual KDevPart *createPartObject(KDevApi *api, QObject *parent, const QStringList &args);
    
  static KInstance *instance();

  
private:

  static KInstance *s_instance;
  
};


#endif
