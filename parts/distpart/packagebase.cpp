#include "packagebase.h"
packageBase::packageBase()
{
   m_AppName = "";
   m_AppVersion = "";
   m_AppRevision = "";
   m_AppURL = "";
   m_AppVendor = "";
   m_AppLicense = "";
   m_AppDescription = "";
   m_AppFileList = "";
}
packageBase::~packageBase()
{
}
void packageBase::setAppName(QString text)
{
}
void packageBase::setAppVersion(QString text)
{
}
void packageBase::setAppRevision(QString text)
{
}
void packageBase::setAppGroup(QString text)
{
}
void packageBase::setAppURL(QString text)
{
}
void packageBase::setAppSummary(QString text)
{
}
void packageBase::setAppVendor(QString text)
{
}
void packageBase::setAppLicense(QString text)
{
}
void packageBase::setAppDescription(QString text)
{
}
void packageBase::setAppFileList(QStringList list)
{
}

QString packageBase::getAppName()
{
}
QString packageBase::getAppVersion()
{
}
QString packageBase::getAppRevision()
{
}
QString packageBase::getAppGroup()
{
}
QString packageBase::getAppURL()
{
}
QString packageBase::getAppSummary()
{
}
QString packageBase::getAppVendor()
{
}
QString packageBase::getAppLicense()
{
}
QString packageBase::getAppDescription()
{
}
QStringList packageBase::getAppFileList()
{
}
