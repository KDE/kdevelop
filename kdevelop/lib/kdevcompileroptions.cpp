#include "kdevcompileroptions.h"

KDevCompilerOptions::KDevCompilerOptions(QWidget *parent, const char *name)
    : QWidget(parent, name)
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
