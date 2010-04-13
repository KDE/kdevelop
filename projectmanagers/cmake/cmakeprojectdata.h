#ifndef CMAKEPROJECTDATA_H
#define CMAKEPROJECTDATA_H

#include <QStringList>
#include "cmaketypes.h"

struct CMakeProjectData
{
    QString projectName;
    QList<Subdirectory> subdirectories;
    QStringList includeDirectories;
    CMakeDefinitions definitions;
    QList<Target> targets;
    
    VariableMap vm;
    MacroMap mm;
    CMakeProperties properties;
};

#endif