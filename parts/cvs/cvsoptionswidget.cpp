#include "domutil.h"
#include "cvsoptionswidget.h"
#include "cvspart.h"
#include <qlineedit.h>

#include "cvs_commonoptions.h"

CvsOptionsWidget::CvsOptionsWidget(CvsPart *part, QWidget *parent, const char *name)
        : CvsOptionsWidgetBase(parent, name) {
    m_part = part;

    readConfig();
}


CvsOptionsWidget::~CvsOptionsWidget() {}


void CvsOptionsWidget::readConfig() {
    QDomDocument &dom = *m_part->projectDom();

    cvs_edit->setText(DomUtil::readEntry(dom,"/kdevcvs/cvsoptions",default_cvs ));
    commit_edit->setText(DomUtil::readEntry(dom,"/kdevcvs/commitoptions",default_commit));
    update_edit->setText(DomUtil::readEntry(dom,"/kdevcvs/updateoptions",default_update));
    add_edit->setText(DomUtil::readEntry(dom,"/kdevcvs/addoptions",default_add));
    remove_edit->setText(DomUtil::readEntry(dom,"/kdevcvs/removeoptions",default_remove));
    diff_edit->setText(DomUtil::readEntry(dom,"/kdevcvs/diffoptions",default_diff));
    log_edit->setText(DomUtil::readEntry(dom,"/kdevcvs/logoptions",default_log));
    rsh_edit->setText(DomUtil::readEntry(dom,"/kdevcvs/rshoptions",default_rsh));

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
    DomUtil::writeEntry(dom, "/kdevcvs/rshoptions", rsh_edit->text());
}


void CvsOptionsWidget::accept() {
    storeConfig();
//    emit configChange();
}
#include "cvsoptionswidget.moc"
