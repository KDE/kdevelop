#ifndef qeditorsearch_h
#define qeditorsearch_h

#include <qobject.h>

#include "koFind.h"
#include "koReplace.h"

class QEditorView;
class QTextParag;

class QEditorFind: public KoFind
{
	Q_OBJECT
public:
	QEditorFind( QEditorView*, const QString&, long, QWidget* =0 );
	virtual ~QEditorFind();
	
	virtual bool validateMatch( const QString&, int, int );
	
public slots:
	void doFind();
	
protected slots:
	void slotHighlight( const QString&, int, int, const QRect& );
	
private:
	QEditorView* m_editor;
	QTextParag* m_currentParag;
};

#endif
