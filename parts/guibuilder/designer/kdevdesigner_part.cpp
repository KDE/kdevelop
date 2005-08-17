
#include "kdevdesigner_part.h"
#include <kaboutdata.h>
#include <kinstance.h>
#include <klocale.h>

KInstance*  KDevDesignerPartFactory::s_instance = 0L;
KAboutData* KDevDesignerPartFactory::s_about = 0L;

KDevDesignerPart::KDevDesignerPart(QWidget *parentWidget, QObject *parent, const char *name, const QStringList &)
  : KParts::ReadWritePart(parent, name),
    m_parentWidget(parentWidget)
{
  setInstance(KDevDesignerPartFactory::instance());
}

KDevDesignerPart::~KDevDesignerPart()
{
}

bool KDevDesignerPart::openFile()
{
  return false;
}

bool KDevDesignerPart::saveFile()
{
  return false;
}

KDevDesignerPartFactory::KDevDesignerPartFactory()
{
}
  
KDevDesignerPartFactory::~KDevDesignerPartFactory()
{
  delete s_instance;
  delete s_about;

  s_instance = 0L;
  s_about = 0L;
}
  
KParts::Part* KDevDesignerPartFactory::createPartObject(QWidget *parentWidget, const char *widgetName,
        QObject *parent, const char *name,
        const char *classname, const QStringList &args)
{
  if (KDevDesignerPart* part = new KDevDesignerPart(parentWidget, parent, name, args))
    {
      if (QLatin1String(classname) == QLatin1String("KParts::ReadOnlyPart"))
          part->setReadWrite(false);

      return part;
    }

  return 0;
}

KInstance* KDevDesignerPartFactory::instance()
{
  if (s_instance == 0)
    {
      s_about = new KAboutData("kdevdesignerpart", I18N_NOOP("KDevDesignerPart"), "0.1");
      s_about->addAuthor("Roberto Raggi", 0, "roberto@kdevelop.org");
      s_about->addAuthor("Trolltech AS", 0, "info@trolltech.com");
      s_instance = new KInstance(s_about);
    }

  return s_instance;
}
  
#include "kdevdesigner_part.moc"
