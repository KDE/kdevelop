#ifndef CMAKETYPES_H
#define CMAKETYPES_H

#include "cmakelistsparser.h"
#include "variablemap.h"

struct Macro
{
    QString name;
    QStringList knownArgs;
    CMakeFileContent code;
    bool isFunction;
};

typedef QHash<QString, Macro> MacroMap;
typedef QHash<QString, QString> Definitions;
typedef QHash<QString, QString> CacheValues;

#endif
