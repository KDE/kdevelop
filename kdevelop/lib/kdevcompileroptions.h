#ifndef _KDEVCOMPILEROPTIONS_H_
#define _KDEVCOMPILEROPTIONS_H_

#include <qwidget.h>


class KDevCompilerOptions : public QWidget
{
    Q_OBJECT

public:
    KDevCompilerOptions( QWidget *parent=0, const char *name=0 );
    ~KDevCompilerOptions();

    virtual void setFlags(const QString &flags);
    virtual QString flags() const;
};

#endif
