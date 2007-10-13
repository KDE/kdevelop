#ifndef CMAKE_DEFINES_H
#define CMAKE_DEFINES_H

#include "cmakelistsparser.h"

struct Macro
{
    QString name;
    QStringList knownArgs;
    CMakeFileContent code;
};

typedef QHash<QString, QStringList> VariableMap;
typedef QHash<QString, Macro> MacroMap;
typedef QPair<QString, QString> Definition;
typedef QList<Definition> DefineList;

#endif
