#include "packagebase.h"
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
    return false;
}

bool packageBase::saveFile(KURL /*theFile*/) {}

bool packageBase::loadFile(KURL /*theFile*/) {}


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
    return (m_dlg->getappNameFormatLineEditText() + "-" + m_dlg->getversionLineEditText() +
	    ((m_dlg->getcustomProjectCheckBoxState() && m_dlg->getbzipCheckBoxState()) ? ".tar.bz2" : ".tar.gz"));
}

QStringList packageBase::getAppFileList() {
    return QStringList();
}
