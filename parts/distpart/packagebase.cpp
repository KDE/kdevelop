#include "packagebase.h"
#include "packagebase.moc"
#include "distpart_part.h"
#include "distpart_widget.h"
#include <qtabwidget.h>
#include "kdevproject.h"

packageBase::packageBase(DistpartPart * part, QString name) {
    m_part = part;
    dir = m_part->project()->projectDirectory();
    m_dlg = m_part->getDlg();
    box = new QVBox(m_dlg->plug_box);
    m_dlg->plug_box->insertTab(box,name);
}

packageBase::~packageBase() {}


QVBox * packageBase::area() {
    return box;
}

bool packageBase::generatePackage(QString &error) {
    Q_UNUSED( error );
    return false;
}

bool packageBase::saveFile(KURL /*theFile*/) { return false; }


bool packageBase::loadFile(KURL /*theFile*/) { return false; }



QString packageBase::getAppName() {
    return m_dlg->getappNameFormatLineEditText();
}

QString packageBase::getAppVersion() {
    return m_dlg->getversionLineEditText();
}

QString packageBase::getAppRevision() {
    return m_dlg->getreleaseLineEditText();
}

QString packageBase::getAppGroup() {
    return m_dlg->getgroupLineEditText();
}

QString packageBase::getAppPackager() {
    return m_dlg->getpackagerLineEditText();
}

QString packageBase::getAppURL() {
    return "";
}

QString packageBase::getAppSummary() {
    return m_dlg->getsummaryLineEditText();
}

QString packageBase::getAppVendor() {
    return m_dlg->getvendorLineEditText();
}

QString packageBase::getAppLicense() {
    return m_dlg->getlicenseLineEditText();
}

QString packageBase::getAppArch() {
    return m_dlg->getarchComboBoxText();
}

QString packageBase::getAppDescription() {
    return m_dlg->getprojectDescriptionMultilineEditText();
}

QString packageBase::getAppChangelog() {
    return m_dlg->getprojectChangelogMultilineEditText();
}

QString packageBase::getAppSource() {
    return m_dlg->getSourceName();
}

QStringList packageBase::getAppFileList() {
    return QStringList();
}

void packageBase::setAppName(QString name) {
    m_dlg->setappNameFormatLineEditText(name);
}

void packageBase::setAppVersion(QString version){
    m_dlg->setversionLineEditText(version);
}

void packageBase::setAppRevision(QString revision){
    m_dlg->setreleaseLineEditText(revision);
}

void packageBase::setAppGroup(QString group){
    m_dlg->setgroupLineEditText(group);
}

void packageBase::setAppPackager(QString packager){
    m_dlg->setpackagerLineEditText(packager);
}

void packageBase::setAppURL(QString url)
{
    Q_UNUSED( url );
}

void packageBase::setAppSummary(QString summary){
    m_dlg->setsummaryLineEditText(summary);
}

void packageBase::setAppVendor(QString vendor){
    m_dlg->setvendorLineEditText(vendor);
}

void packageBase::setAppLicense(QString licence){
    m_dlg->setlicenseLineEditText(licence);
}

void packageBase::setAppArch(QString arch){
    Q_UNUSED( arch );
}

void packageBase::setAppDescription(QString description){
    m_dlg->setprojectDescriptionMultilineEditText(description);
}

void packageBase::setAppChangelog(QString changelog){
    m_dlg->setprojectChangelogMultilineEditText(changelog);
}
