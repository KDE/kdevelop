#include "domutil.h"
#include "cvsoptionswidget.h"
#include <qlineedit.h>
#include <qlabel.h>
//#include <qvbox.h>
#include <qlayout.h>

#include <kdialog.h>

#include "serverconfigurationwidget.h"

#include "cvsoptions.h"

CvsOptionsWidget::CvsOptionsWidget( QWidget *parent, const char *name )
    : CvsOptionsWidgetBase( parent, name )
{
    readConfig();
}

CvsOptionsWidget::~CvsOptionsWidget() {}


void CvsOptionsWidget::readConfig() {
    CvsOptions *options = CvsOptions::instance();

    m_cvsEdit->setText( options->cvs() );
    m_commitEdit->setText( options->commit() );
    m_updateEdit->setText( options->update() );
    m_addEdit->setText( options->add() );
    m_removeEdit->setText( options->remove() );
    m_diffEdit->setText( options->diff() );
    m_logEdit->setText( options->log() );
    m_rshEdit->setText( options->rsh() );
}


void CvsOptionsWidget::storeConfig() {
    CvsOptions *options = CvsOptions::instance();

    options->setCvs( cvs() );
    options->setCommit( commit() );
    options->setUpdate( update() );
    options->setAdd( add() );
    options->setRemove( remove() );
    options->setDiff( diff() );
    options->setLog( log() );
    options->setRsh( rsh() );

}

void CvsOptionsWidget::accept() {
    storeConfig();
//    emit configChange();
}

QString CvsOptionsWidget::cvs() const
{
    return m_cvsEdit->text();
}

QString CvsOptionsWidget::commit() const
{
    return m_commitEdit->text();
}

QString CvsOptionsWidget::update() const
{
    return m_updateEdit->text();
}

QString CvsOptionsWidget::add() const
{
    return m_addEdit->text();
}

QString CvsOptionsWidget::remove() const
{
    return m_removeEdit->text();
}

QString CvsOptionsWidget::diff() const
{
    return m_diffEdit->text();
}

QString CvsOptionsWidget::log() const
{
    return m_logEdit->text();
}

QString CvsOptionsWidget::rsh() const
{
    return m_rshEdit->text();
}

#include "cvsoptionswidget.moc"
