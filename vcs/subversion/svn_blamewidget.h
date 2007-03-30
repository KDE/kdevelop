#ifndef SVN_BLAMEWIDGET_H
#define SVN_BLAMEWIDGET_H

#include <qwidget.h>
#include <qdialog.h>
#include <qvaluelist.h>
class QVBoxLayout;
class QListView;
class QGridLayout;
class QPushButton;
class QStringList;

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

class SvnBlameFileSelectDlg : public QDialog {
    Q_OBJECT
public:
    SvnBlameFileSelectDlg( QWidget *parent = 0L );
    virtual ~SvnBlameFileSelectDlg();
    void setCandidate( QStringList *modifies );
    QString selected();
    
protected:
    virtual void accept();
    
private:
    QGridLayout *m_layout;
    QListView *m_view;
    QPushButton *m_okBtn;
    QPushButton *m_cancelBtn;
    
    QStringList *m_candidates;
    QString m_selected;
};

#endif
