#ifndef SVN_BLAMEWIDGET_H
#define SVN_BLAMEWIDGET_H

#include <qwidget.h>
#include <qvaluelist.h>
class QVBoxLayout;
class QListView;

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

class SvnBlameWidget : public QWidget {
	Q_OBJECT
public:
	SvnBlameWidget( QWidget * parent = 0, const char * name = 0, bool modal = FALSE, WFlags f = 0 );
	virtual ~SvnBlameWidget();
// 	void copyBlameData( QMap<unsigned int, SvnBlameHolder> *blamelist );
	void copyBlameData( QValueList<SvnBlameHolder> *blamelist );
	void show();
	QListView* outView();
protected:
    QValueList <SvnBlameHolder> m_blamelist;
    
    QVBoxLayout *m_layout;
    QListView *m_listView;
// 	QMap <unsigned int, SvnBlameHolder> m_blamelist;		
};

#endif
