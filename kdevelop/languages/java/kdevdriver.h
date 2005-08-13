
#ifndef __kdevdriver_h
#define __kdevdriver_h

#include "javasupportpart.h"

#include <kdevproject.h>
#include <kdeversion.h>

#include "driver.h"
#include <qprocess.h>

#include <cstdlib>
#include <unistd.h>

class KDevDriver: public Driver
{
public:
    KDevDriver( JavaSupportPart* javaSupport );
    JavaSupportPart* javaSupport();
    void setupProject();

protected:
    void setupLexer( JavaLexer* lexer );

private:
    JavaSupportPart* m_javaSupport;
};

#endif

