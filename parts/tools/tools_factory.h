#ifndef __TOOLS_FACTORY_H__
#define __TOOLS_FACTORY_H__


#include <kdevfactory.h>


class ToolsFactory : public KDevFactory
  {
    Q_OBJECT

public:

    ToolsFactory(QObject *parent = 0, const char *name = 0);
    ~ToolsFactory();

    virtual KDevPart *createPartObject(KDevApi *api, QObject *parent, const QStringList &args);

    static KInstance *instance();


private:

    static KInstance *s_instance;

  };


#endif
