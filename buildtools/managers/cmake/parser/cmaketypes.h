#ifndef CMAKE_DEFINES_H
#define CMAKE_DEFINES_H

struct Macro
{
    QString name;
    QStringList knownArgs;
    CMakeFileContent code;
};

typedef QHash<QString, QStringList> VariableMap;
typedef QHash<QString, Macro> MacroMap;

#endif
