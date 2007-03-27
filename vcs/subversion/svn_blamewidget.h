#ifndef SVN_BLAMEWIDGET_H
#define SVN_BLAMEWIDGET_H

#include "svn_blamewidgetbase.h"

class SvnBlameHolder {
public:
// 	SvnBlameHolder(){};
// 	~SvnBlameHolder(){};
	unsigned int line;
	long int rev;
	QString date;
	QString author;
	QString content;	
};	

class QListView;

class SvnBlameWidget : public SvnBlameWidgetBase {
	Q_OBJECT
public:
	SvnBlameWidget( QWidget * parent = 0, const char * name = 0, bool modal = FALSE, WFlags f = 0 );
	~SvnBlameWidget();
// 	void copyBlameData( QMap<unsigned int, SvnBlameHolder> *blamelist );
	void copyBlameData( QValueList<SvnBlameHolder> *blamelist );
	int exec();
	QListView* outView();
protected:
	QValueList <SvnBlameHolder> m_blamelist;
// 	QMap <unsigned int, SvnBlameHolder> m_blamelist;		
};

#endif
