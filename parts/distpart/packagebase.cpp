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
bool packageBase::generatePackage(QString &/*error*/)
{
    return false;
}
bool packageBase::saveFile(KURL /*theFile*/)
{
}
bool packageBase::loadFile(KURL /*theFile*/)
{
}
void packageBase::setAppName(QString /*text*/)
{
}
void packageBase::setAppVersion(QString /*text*/)
{
}
void packageBase::setAppRevision(QString /*text*/)
{
}
void packageBase::setAppGroup(QString /*text*/)
{
}
void packageBase::setAppURL(QString /*text*/)
{
}
void packageBase::setAppSummary(QString /*text*/)
{
}
void packageBase::setAppVendor(QString /*text*/)
{
}
void packageBase::setAppLicense(QString /*text*/)
{
}
void packageBase::setAppDescription(QString /*text*/)
{
}
void packageBase::setAppFileList(QStringList /*list*/)
{
}

QString packageBase::getAppName()
{
    return QString::null;
}
QString packageBase::getAppVersion()
{
    return QString::null;
}
QString packageBase::getAppRevision()
{
    return QString::null;
}
QString packageBase::getAppGroup()
{
    return QString::null;
}
QString packageBase::getAppURL()
{
    return QString::null;
}
QString packageBase::getAppSummary()
{
    return QString::null;
}
QString packageBase::getAppVendor()
{
    return QString::null;
}
QString packageBase::getAppLicense()
{
    return QString::null;
}
QString packageBase::getAppDescription()
{
    return QString::null;
}
QStringList packageBase::getAppFileList()
{
    return QStringList();
}
