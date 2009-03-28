#ifndef CMAKEPROJECTDATA_H
#define CMAKEPROJECTDATA_H

#include <QStringList>
#include "cmaketypes.h"

struct CMakeProjectData
{
    QString projectName;
    QStringList subdirectories;
    QStringList includeDirectories;
    CMakeDefinitions definitions;
    QList<Target> targets;
    QMap<QString, CMakeFunctionDesc> folderDeclarations;
    
    VariableMap vm;
    MacroMap mm;
};

#endif