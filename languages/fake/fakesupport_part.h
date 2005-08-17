#ifndef FAKESUPPORT_PART_H
#define FAKESUPPORT_PART_H

#include <kdevlanguagesupport.h>

class FakeLanguageSupport: public KDevLanguageSupport
{
    Q_OBJECT
public:
    FakeLanguageSupport(QObject *parent, const char *name, const QStringList &args);
    virtual ~FakeLanguageSupport();
};

#endif // FAKESUPPORT_PART_H
