#ifndef CMAKEPROJECTDATA_H
#define CMAKEPROJECTDATA_H

#include <QStringList>
#include "cmaketypes.h"

struct CMakeProjectData
{
    QString projectName;
    QVector<Subdirectory> subdirectories;
    QVector<Target> targets;
    QVector<Test> testSuites;
    
    VariableMap vm;
    MacroMap mm;
    CMakeProperties properties;
    CacheValues cache;
    QStringList modulePath;
    QHash<QString,QString> targetAlias;
    
    void clear() { vm.clear(); mm.clear(); properties.clear(); cache.clear(); targetAlias.clear(); }
};

#endif
