#include "domutil.h"
#include "cvsoptionswidget.h"
#include "cvspart.h"
#include <qlineedit.h>

#include "cvsoptions.h"

CvsOptionsWidget::CvsOptionsWidget(CvsPart *part, QWidget *parent, const char *name)
        : CvsOptionsWidgetBase(parent, name) {
    m_part = part;

    readConfig();
}


CvsOptionsWidget::~CvsOptionsWidget() {}


void CvsOptionsWidget::readConfig() {
	CvsOptions *options = CvsOptions::instance();
//	options->load( *m_part->projectDom() );

    cvs_edit->setText( options->cvs() );
    commit_edit->setText( options->commit() );
    update_edit->setText( options->update() );
    add_edit->setText( options->add() );
    remove_edit->setText( options->remove() );
    diff_edit->setText( options->diff() );
    log_edit->setText( options->log() );
    rsh_edit->setText( options->rsh() );
}


void CvsOptionsWidget::storeConfig() {
	CvsOptions *options = CvsOptions::instance();

    options->setCvs( cvs_edit->text() );
    options->setCommit( commit_edit->text() );
    options->setUpdate( update_edit->text() );
    options->setAdd( add_edit->text() );
    options->setRemove( remove_edit->text() );
    options->setDiff( diff_edit->text() );
    options->setLog( log_edit->text() );
    options->setRsh( rsh_edit->text() );

//	options->save( *m_part->projectDom() );
}


void CvsOptionsWidget::accept() {
    storeConfig();
//    emit configChange();
}
#include "cvsoptionswidget.moc"
