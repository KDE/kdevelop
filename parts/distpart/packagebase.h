#ifndef __PACKAGEBASE_H__
#define __PACKAGEBASE_H__

#include <qwidget.h>

#include "distpart_ui.h"
#include <kurl.h>
#include <qstringlist.h>
#include <qvbox.h>

class DistpartPart;
class DistpartDialog;

class packageBase : public QWidget
{
    Q_OBJECT
    
public:
   packageBase(DistpartPart * part, QString name);
   virtual ~packageBase();

   QVBox * area();
   
// Generic accessors and mutators for child classes
   virtual bool generatePackage(QString &errorCode);
   virtual bool loadFile(KURL theFile);
   virtual bool saveFile(KURL theFile);

   virtual QString getAppName();
   virtual QString getAppVersion();
   virtual QString getAppRevision();
   virtual QString getAppGroup();
   virtual QString getAppPackager();
   virtual QString getAppURL();
   virtual QString getAppSummary();
   virtual QString getAppVendor();
   virtual QString getAppLicense();
   virtual QString getAppArch();
   virtual QString getAppDescription();
   virtual QString getAppChangelog();
   virtual QString getAppSource();
   virtual QStringList getAppFileList();
   
   virtual void setAppName(QString);
   virtual void setAppVersion(QString);
   virtual void setAppRevision(QString);
   virtual void setAppGroup(QString);
   virtual void setAppPackager(QString);
   virtual void setAppURL(QString);
   virtual void setAppSummary(QString);
   virtual void setAppVendor(QString);
   virtual void setAppLicense(QString);
   virtual void setAppArch(QString);
   virtual void setAppDescription(QString);
   virtual void setAppChangelog(QString);
   
   private:
   DistpartPart * m_part;
   DistpartDialog * m_dlg;
   QVBox * box;
   
   protected:
   QString dir;
   
};

#endif
