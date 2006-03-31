#ifndef KDEVDESIGNER_PART_H
#define KDEVDESIGNER_PART_H

#include <kparts/part.h>
#include <kparts/factory.h>

class KDevDesignerPart: public KParts::ReadWritePart
{
  Q_OBJECT
public:
  KDevDesignerPart(QWidget *parentWidget, QObject *parent, const char *name, const QStringList &args);
  virtual ~KDevDesignerPart();

protected:
  virtual bool openFile();
  virtual bool saveFile();

private:
  QWidget *m_parentWidget;
};

class KAboutData;
class KInstance;


class KDevDesignerPartFactory : public KParts::Factory
{
  Q_OBJECT
public:
  KDevDesignerPartFactory();
  virtual ~KDevDesignerPartFactory();

  virtual KParts::Part* createPartObject(QWidget *parentWidget, const char *widgetName,
        QObject *parent, const char *name,
        const char *classname, const QStringList &args);

  static KInstance* instance();

private:
  static KInstance* s_instance;
  static KAboutData* s_about;
};

#endif // KDEVDESIGNER_PART_H
