#ifndef _GCCOPTIONS_H_
#define _GCCOPTIONS_H_

#include <qlist.h>
#include "kdevcompileroptions.h"


class GeneralTab;
class OptimizationTab;
class Warnings1Tab;
class Warnings2Tab;


class GccOptions : public KDevCompilerOptions
{
    Q_OBJECT

public:
    GccOptions( bool cpp, QWidget *parent=0, const char *name=0 );
    ~GccOptions();

protected:
    virtual void setFlags(const QString &flags);
    virtual QString flags() const;

private:
    GeneralTab *general;
    OptimizationTab *optimization;
    Warnings1Tab *warnings1;
    Warnings2Tab *warnings2;
};

#endif
