#ifndef _KDEVCOMPILEROPTIONS_H_
#define _KDEVCOMPILEROPTIONS_H_

#include <kdialogbase.h>


class KDevCompilerOptions : public KDialogBase
{
    Q_OBJECT

public:
    KDevCompilerOptions( int dialogFace, const QString &caption,
                         QWidget *parent=0, const char *name=0 );
    ~KDevCompilerOptions();

    virtual void setFlags(const QString &flags);
    virtual QString flags() const;
};

#endif
