#ifndef BACKGROUNDPARSER_H
#define BACKGROUNDPARSER_H

#include <qthread.h>
#include <qstring.h>

class ProblemReporter;

class BackgroundParser: public QThread{
public:
    BackgroundParser( ProblemReporter* reporter,
                      const QString& source,
                      const QString& filename );
    virtual ~BackgroundParser();

    virtual void run();

private:
    ProblemReporter* m_reporter;
    QString m_source;
    QString m_fileName;
};

#endif
