#include "packagebase.h"

packageBase::packageBase() {

}

packageBase::~packageBase() {}

QString packageBase::getAppName() {
    return AppName;
}

QString packageBase::getAppVersion() {
    return AppVersion;
}

QString packageBase::getAppRevision() {
    return AppRevision;
}

QString packageBase::getAppGroup() {
    return AppGroup;
}

QString packageBase::getAppPackager() {
    return AppPackager;
}

QString packageBase::getAppURL() {
    return AppURL;
}

QString packageBase::getAppSummary() {
    return AppSummary;
}

QString packageBase::getAppVendor() {
    return AppVendor;
}

QString packageBase::getAppLicense() {
    return AppLicense;
}

QString packageBase::getAppArch() {
    return AppArch;
}

QString packageBase::getAppDescription() {
    return AppDescription;
}

QString packageBase::getAppChangelog() {
    return AppChangelog;
}

QString packageBase::getAppSource() {
    return AppSource;
}

QStringList packageBase::getAppFileList() {
    return AppFileList;
}

void packageBase::setAppName(const QString& name) {
    AppName = name;
}

void packageBase::setAppVersion(const QString& version){
   AppVersion = version;
}

void packageBase::setAppRevision(const QString& revision){
    AppRevision = revision;
}

void packageBase::setAppGroup(const QString& group){
   AppGroup = group;
}

void packageBase::setAppPackager(const QString& packager){
    AppPackager = packager;
}

void packageBase::setAppURL(const QString& url)
{
    AppURL = url;
}

void packageBase::setAppSummary(const QString& summary){
    AppSummary = summary;
}

void packageBase::setAppVendor(const QString& vendor){
    AppVendor = vendor;
}

void packageBase::setAppLicense(const QString& licence){
    AppLicense = licence;
}

void packageBase::setAppArch(const QString& arch){
    AppArch = arch;
}

void packageBase::setAppDescription(const QString& description){
    AppDescription = description;
}

void packageBase::setAppChangelog(const QString& changelog){
    AppChangelog = changelog;
}

void packageBase::setAppFileList( const QStringList & list )
{
	AppFileList = list;
}
