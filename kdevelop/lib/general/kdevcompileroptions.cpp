#include "kdevcompileroptions.h"

KDevCompilerOptions::KDevCompilerOptions( int dialogFace, const QString &caption,
                                          QWidget *parent, const char *name )
    : KDialogBase(dialogFace, caption, Ok|Cancel, Ok, parent, name, true)
{
}

KDevCompilerOptions::~KDevCompilerOptions()
{
}

void KDevCompilerOptions::setFlags(const QString &flags)
{
}

QString KDevCompilerOptions::flags() const
{
    return QString();
}
