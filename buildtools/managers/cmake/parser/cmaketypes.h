#ifndef CMAKE_DEFINES_H
#define CMAKE_DEFINES_H

#include "cmakelistsparser.h"
#include "variablemap.h"

struct Macro
{
    QString name;
    QStringList knownArgs;
    CMakeFileContent code;
};

typedef QHash<QString, Macro> MacroMap;
typedef QHash<QString, QString> Definitions;
#endif
