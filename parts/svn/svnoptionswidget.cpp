#include "domutil.h"
#include "svnoptionswidget.h"
#include "svnpart.h"
#include <qlineedit.h>

#include "svnoptions.h"

SvnOptionsWidget::SvnOptionsWidget(SvnPart *part, QWidget *parent, const char *name)
        : SvnOptionsWidgetBase(parent, name) {
    m_part = part;

    readConfig();
}


SvnOptionsWidget::~SvnOptionsWidget() {}


void SvnOptionsWidget::readConfig() {
	SvnOptions *options = SvnOptions::instance();
//	options->load( *m_part->projectDom() );

    svn_edit->setText( options->svn() );
    commit_edit->setText( options->commit() );
    update_edit->setText( options->update() );
    add_edit->setText( options->add() );
    remove_edit->setText( options->remove() );
    diff_edit->setText( options->diff() );
    log_edit->setText( options->log() );
    rsh_edit->setText( options->rsh() );
}


void SvnOptionsWidget::storeConfig() {
	SvnOptions *options = SvnOptions::instance();

    options->setSvn( svn_edit->text() );
    options->setCommit( commit_edit->text() );
    options->setUpdate( update_edit->text() );
    options->setAdd( add_edit->text() );
    options->setRemove( remove_edit->text() );
    options->setDiff( diff_edit->text() );
    options->setLog( log_edit->text() );
    options->setRsh( rsh_edit->text() );

//	options->save( *m_part->projectDom() );
}


void SvnOptionsWidget::accept() {
    storeConfig();
//    emit configChange();
}
#include "svnoptionswidget.moc"
