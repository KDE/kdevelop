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
//    : QWidget( parent, name )
{
/*
	m_cvsOptionsWidgetLayout = new QVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint(), "m_cvsOptionsWidgetLayout");

	m_cfgWidget = new ServerConfigurationWidget( this, "serverconfigurationwidget" );
    m_cfgWidget->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, m_cfgWidget->sizePolicy().hasHeightForWidth() ) );
	m_cvsOptionsWidgetLayout->addWidget( m_cfgWidget );

    m_cvsLabel = new QLabel( this, "cvsLabel" );
    m_cvsOptionsWidgetLayout->addWidget( m_cvsLabel );

    m_cvsEdit = new QLineEdit( this, "cvsEdit" );
    m_cvsOptionsWidgetLayout->addWidget( m_cvsEdit );

    m_commitLabel = new QLabel( this, "commitLabel" );
    m_cvsOptionsWidgetLayout->addWidget( m_commitLabel );

    m_commitEdit = new QLineEdit( this, "commitEdit" );
    m_cvsOptionsWidgetLayout->addWidget( m_commitEdit );

    m_updateLabel = new QLabel( this, "updateLabel" );
    m_cvsOptionsWidgetLayout->addWidget( m_updateLabel );

    m_updateEdit = new QLineEdit( this, "updateEdit" );
    m_cvsOptionsWidgetLayout->addWidget( m_updateEdit );

    m_addLabel = new QLabel( this, "addLabel" );
    m_cvsOptionsWidgetLayout->addWidget( m_addLabel );

    m_addEdit = new QLineEdit( this, "addEdit" );
    m_cvsOptionsWidgetLayout->addWidget( m_addEdit );

    m_removeLabel = new QLabel( this, "removeLabel" );
    m_cvsOptionsWidgetLayout->addWidget( m_removeLabel );

    m_removeEdit = new QLineEdit( this, "removeEdit" );
    m_cvsOptionsWidgetLayout->addWidget( m_removeEdit );

    m_diffLabel = new QLabel( this, "diffLabel" );
    m_cvsOptionsWidgetLayout->addWidget( m_diffLabel );

    m_diffEdit = new QLineEdit( this, "diffEdit" );
	m_cvsOptionsWidgetLayout->addWidget( m_diffEdit );

    m_logLabel = new QLabel( this, "logLabel" );
    m_cvsOptionsWidgetLayout->addWidget( m_logLabel );

    m_logEdit = new QLineEdit( this, "logEdit" );
    m_cvsOptionsWidgetLayout->addWidget( m_logEdit );

    m_rshLabel = new QLabel( this, "rshLabel" );
    m_cvsOptionsWidgetLayout->addWidget( m_rshLabel );

    m_rshEdit = new QLineEdit( this, "rshEdit" );
    m_cvsOptionsWidgetLayout->addWidget( m_rshEdit );

    resize( QSize(620, 240).expandedTo(minimumSizeHint()) );
//    clearWState( WState_Polished );
*/
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
