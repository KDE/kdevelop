#ifndef _KMDI_DOCUMENT_VIEW_TAB_WIDGET_H_
#define _KMDI_DOCUMENT_VIEW_TAB_WIDGET_H_

#include <ktabwidget.h>

class KMdiDocumentViewTabWidget:
 public KTabWidget
{
	Q_OBJECT
public:
	KMdiDocumentViewTabWidget(QWidget* parent, const char* name=0);
	~KMdiDocumentViewTabWidget();

	virtual void addTab ( QWidget * child, const QString & label );
	virtual void addTab ( QWidget * child, const QIconSet & iconset, const QString & label );
	virtual void addTab ( QWidget * child, QTab * tab );
	virtual void insertTab ( QWidget * child, const QString & label, int index = -1 );
	virtual void insertTab ( QWidget * child, const QIconSet & iconset, const QString & label, int index = -1 );
	virtual void insertTab ( QWidget * child, QTab * tab, int index = -1 );
	virtual void removePage ( QWidget * w );

private slots:
	void closeTab(QWidget* w);
public slots:
	void updateIconInView(QWidget*,QPixmap);
	void updateCaptionInView(QWidget*,const QString&);
};



#endif
