
#ifndef __kdevdriver_h
#define __kdevdriver_h

#include "cppsupportpart.h"

#include <kdevproject.h>
#include <kdeversion.h>

#include <driver.h>
#include <qprocess.h>

#include <cstdlib>
#include <unistd.h>

class KDevDriver: public Driver
{
public:
    KDevDriver( CppSupportPart* cppSupport );
    CppSupportPart* cppSupport();
    void setupProject();
    // setup the preprocessor
    // code provided by Reginald Stadlbauer <reggie@trolltech.com>
    void setup();
private:
    CppSupportPart* m_cppSupport;
};

#endif

