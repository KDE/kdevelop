
#ifndef __cppsupport_events_h
#define __cppsupport_events_h

#include "parser.h"

#include <qevent.h>
#include <qvaluelist.h>

const int Event_FoundProblems = QEvent::User + 1000;

class FoundProblemsEvent: public QCustomEvent
{
public:
    FoundProblemsEvent( QString fileName, const QValueList<Problem>& problems )
	: QCustomEvent(Event_FoundProblems), m_fileName( fileName ), m_problems( problems ) {}
    
    QString fileName() const { return m_fileName; }
    QValueList<Problem> problems() const { return m_problems; }
    
private:
    QString m_fileName;
    QValueList<Problem> m_problems;
};
    

#endif // __cppsupport_events_h
