#include "domutil.h"
#include "cvsoptionswidget.h"
#include "cvspart.h"
#include <qlineedit.h>

CvsOptionsWidget::CvsOptionsWidget(CvsPart *part, QWidget *parent, const char *name)
        : CvsOptionsWidgetBase(parent, name) {
    m_part = part;

    readConfig();
}


CvsOptionsWidget::~CvsOptionsWidget() {}


void CvsOptionsWidget::readConfig() {
    QDomDocument &dom = *m_part->projectDom();

    cvs_edit->setText(DomUtil::readEntry(dom,"/kdevcvs/cvsoptions",m_part->default_cvs ));
    commit_edit->setText(DomUtil::readEntry(dom,"/kdevcvs/commitoptions",m_part->default_commit));
    update_edit->setText(DomUtil::readEntry(dom,"/kdevcvs/updateoptions",m_part->default_update));
    add_edit->setText(DomUtil::readEntry(dom,"/kdevcvs/addoptions",m_part->default_add));
    remove_edit->setText(DomUtil::readEntry(dom,"/kdevcvs/removeoptions",m_part->default_remove));
    diff_edit->setText(DomUtil::readEntry(dom,"/kdevcvs/diffoptions",m_part->default_diff));
    log_edit->setText(DomUtil::readEntry(dom,"/kdevcvs/logoptions",m_part->default_log));

}


void CvsOptionsWidget::storeConfig() {
    QDomDocument &dom = *m_part->projectDom();

    DomUtil::writeEntry(dom, "/kdevcvs/cvsoptions", cvs_edit->text());
    DomUtil::writeEntry(dom, "/kdevcvs/commitoptions", commit_edit->text());
    DomUtil::writeEntry(dom, "/kdevcvs/updateoptions", update_edit->text());
    DomUtil::writeEntry(dom, "/kdevcvs/addoptions", add_edit->text());
    DomUtil::writeEntry(dom, "/kdevcvs/removeoptions", remove_edit->text());
    DomUtil::writeEntry(dom, "/kdevcvs/diffoptions", diff_edit->text());
    DomUtil::writeEntry(dom, "/kdevcvs/logoptions", log_edit->text());
}


void CvsOptionsWidget::accept() {
    storeConfig();
//    emit m_part->configChange();
}
