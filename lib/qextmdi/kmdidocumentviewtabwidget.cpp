#include <ktabbar.h>
#include "kmdidocumentviewtabwidget.h"

KMdiDocumentViewTabWidget::KMdiDocumentViewTabWidget(QWidget* parent, const char* name):KTabWidget(parent,name) {
//	QTabBar *bar=tabBar();
	tabBar()->hide();
	setHoverCloseButton(true);
        connect(this, SIGNAL(closeRequest(QWidget*)), this, SLOT(closeTab(QWidget*)));
}
	
KMdiDocumentViewTabWidget::~KMdiDocumentViewTabWidget() {
}

void KMdiDocumentViewTabWidget::closeTab(QWidget* w) {
	w->close();
}
void KMdiDocumentViewTabWidget::addTab ( QWidget * child, const QString & label ) {
	KTabWidget::addTab(child,label);
    showPage(child);
	if (count()>1) tabBar()->show();
}

void KMdiDocumentViewTabWidget::addTab ( QWidget * child, const QIconSet & iconset, const QString & label ) {
	KTabWidget::addTab(child,iconset,label);
    showPage(child);
	if (count()>1) tabBar()->show();
}

void KMdiDocumentViewTabWidget::addTab ( QWidget * child, QTab * tab ) {
	KTabWidget::addTab(child,tab);
    showPage(child);
	if (count()>1) tabBar()->show();
}

void KMdiDocumentViewTabWidget::insertTab ( QWidget * child, const QString & label, int index) {
	KTabWidget::insertTab(child,label,index);
    showPage(child);
	if (count()>1) tabBar()->show();
}

void KMdiDocumentViewTabWidget::insertTab ( QWidget * child, const QIconSet & iconset, const QString & label, int index ) {
	KTabWidget::insertTab(child,iconset,label,index);
    showPage(child);
	if (count()>1) tabBar()->show();
}

void KMdiDocumentViewTabWidget::insertTab ( QWidget * child, QTab * tab, int index) {
	KTabWidget::insertTab(child,tab,index);
    showPage(child);
	if (count()>1) tabBar()->show();
}

void KMdiDocumentViewTabWidget::removePage ( QWidget * w ) {
	KTabWidget::removePage(w);
	if (count()<2) tabBar()->hide();
}


void KMdiDocumentViewTabWidget::updateView(QWidget* w,QPixmap icon,const QString& caption) {
	changeTab(w,icon,caption);
}
#ifndef NO_INCLUDE_MOCFILES
#include "kmdidocumentviewtabwidget.moc"
#endif
