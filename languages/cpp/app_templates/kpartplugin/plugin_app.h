
#ifndef _PLUGIN_%{APPNAMEUC}_H_
#define _PLUGIN_%{APPNAMEUC}_H_

#include <kparts/plugin.h>
#include <kparts/factory.h>

class Plugin%{APPNAME} : public KParts::Plugin
{
  Q_OBJECT
public:
  Plugin%{APPNAME}( QObject* parent = 0, const char* name = 0 );
  virtual ~Plugin%{APPNAME}();

public slots:
  void slotAction();
};

class %{APPNAME}Factory : public KParts::Factory
{
  Q_OBJECT
public:
  %{APPNAME}Factory();
  virtual ~%{APPNAME}Factory();

  virtual QObject* createObject( QObject* parent = 0, const char* pname = 0, const char* name = "QObject", const QStringList &args = QStringList() );

  static KInstance* instance();

private:
  static KInstance* s_instance;
};

#endif // _PLUGIN_%{APPNAMEUC}_H_
