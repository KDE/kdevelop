#ifndef __PLUGINCONTROLLER_H__
#define __PLUGINCONTROLLER_H__


class KDevPart;
class KService;


class PluginController
{

public:

  ~PluginController();

  static void createInstance();
  static PluginController *getInstance();


  KDevPart *loadPlugin(KService *service, const char *className, QObject *parent);


protected:

  PluginController();


private:

  void loadGlobalPlugins();
  void loadDefaultParts();

  void integratePart(KDevPart *part);

  static PluginController *s_instance;

};


#endif
