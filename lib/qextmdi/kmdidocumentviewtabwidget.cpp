#include <qtabbar.h>
#include "kmdidocumentviewtabwidget.h"

KMdiDocumentViewTabWidget::KMdiDocumentViewTabWidget(QWidget* parent, const char* name):QTabWidget(parent,name) {
//	QTabBar *bar=tabBar();
	tabBar()->hide();
//	setHoverCloseButton(true);
}
	
KMdiDocumentViewTabWidget::~KMdiDocumentViewTabWidget() {
}

void KMdiDocumentViewTabWidget::addTab ( QWidget * child, const QString & label ) {
	QTabWidget::addTab(child,label);
	if (count()>1) tabBar()->show();
}

void KMdiDocumentViewTabWidget::addTab ( QWidget * child, const QIconSet & iconset, const QString & label ) {
	QTabWidget::addTab(child,iconset,label);
	if (count()>1) tabBar()->show();
}

void KMdiDocumentViewTabWidget::addTab ( QWidget * child, QTab * tab ) {
	QTabWidget::addTab(child,tab);
	if (count()>1) tabBar()->show();
}

void KMdiDocumentViewTabWidget::insertTab ( QWidget * child, const QString & label, int index) {
	QTabWidget::insertTab(child,label,index);
	if (count()>1) tabBar()->show();
	
}

void KMdiDocumentViewTabWidget::insertTab ( QWidget * child, const QIconSet & iconset, const QString & label, int index ) {
	QTabWidget::insertTab(child,iconset,label,index);
	if (count()>1) tabBar()->show();
}

void KMdiDocumentViewTabWidget::insertTab ( QWidget * child, QTab * tab, int index) {
	QTabWidget::insertTab(child,tab,index);
	if (count()>1) tabBar()->show();
}

void KMdiDocumentViewTabWidget::removePage ( QWidget * w ) {
	QTabWidget::removePage(w);
	if (count()<2) tabBar()->hide();
}

#ifndef NO_INCLUDE_MOCFILES
#include "kmdidocumentviewtabwidget.moc"
#endif
