#include "domutil.h"
#include "svnoptionswidget.h"
#include "svn_part.h"
#include <qcheckbox.h>

	svnOptionsWidget::svnOptionsWidget(svnPart *part, QWidget *parent, const char *name)
: svnoptions(parent, name) 
{
	m_part = part;
	readConfig();
}

svnOptionsWidget::~svnOptionsWidget() {}

void svnOptionsWidget::readConfig() {
	QDomDocument &dom = *m_part->projectDom();

	force->setChecked(DomUtil::readBoolEntry(dom,"/kdevsvn/force",m_part->force));
	recurse->setChecked(DomUtil::readBoolEntry(dom,"/kdevsvn/recurse",m_part->recursive));
	verbose->setChecked(DomUtil::readBoolEntry(dom,"/kdevsvn/verbose",m_part->verbose));
}

void svnOptionsWidget::storeConfig() {
	QDomDocument &dom = *m_part->projectDom();

	DomUtil::writeBoolEntry(dom, "/kdevsvn/force", force->isChecked());
	DomUtil::writeBoolEntry(dom, "/kdevsvn/recurse", recurse->isChecked());
	DomUtil::writeBoolEntry(dom, "/kdevsvn/verbose", verbose->isChecked());
}

void svnOptionsWidget::accept() {
	storeConfig();
	m_part->readConf();
}
/* vim: set ai ts=8 sw=8 : */
