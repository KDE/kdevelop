#ifndef __PACKAGEBASE_H__
#define __PACKAGEBASE_H__

#include <kurl.h>
#include <qstringlist.h>

class packageBase
{
public:
   packageBase();
   virtual ~packageBase();

// Generic accessors and mutators for child classes
   virtual bool generatePackage(QString &errorCode) = 0;
   virtual bool loadFile(KURL theFile) = 0;

   virtual void setAppName(QString text = "");
   virtual void setAppVersion(QString text = "");
   virtual void setAppRevision(QString text = "");
   virtual void setAppGroup(QString text = "");
   virtual void setAppURL(QString text = "");
   virtual void setAppSummary(QString text = "");
   virtual void setAppVendor(QString text = "");
   virtual void setAppLicense(QString text = "");
   virtual void setAppDescription(QString text = "");
   virtual void setAppFileList(QStringList list);

   virtual QString getAppName();
   virtual QString getAppVersion();
   virtual QString getAppRevision();
   virtual QString getAppGroup();
   virtual QString getAppURL();
   virtual QString getAppSummary();
   virtual QString getAppVendor();
   virtual QString getAppLicense();
   virtual QString getAppDescription();
   virtual QStringList getAppFileList();
private:
   QString m_AppName;
   QString m_AppVersion;
   QString m_AppRevision;
   QString m_AppURL;
   QString m_AppVendor;
   QString m_AppLicense;
   QString m_AppDescription;
   QStringList m_AppFileList;
};

#endif
