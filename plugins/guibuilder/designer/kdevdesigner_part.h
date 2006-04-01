#ifndef KDEVDESIGNER_PART_H
#define KDEVDESIGNER_PART_H

#include <kparts/part.h>

class KDevDesignerPart: public KParts::ReadWritePart
{
  Q_OBJECT
public:
  KDevDesignerPart(QWidget *parentWidget, const char* widgetName, 
                   QObject *parent, const char *name, const QStringList &args);
  virtual ~KDevDesignerPart();

protected:
  virtual bool openFile();
  virtual bool saveFile();

private:
  QWidget *m_parentWidget;
};



#endif // KDEVDESIGNER_PART_H
