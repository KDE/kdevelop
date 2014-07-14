#ifndef CMAKEPROJECTDATA_H
#define CMAKEPROJECTDATA_H

#include <QStringList>
#include "cmaketypes.h"
#include <project/path.h>

/**
 * Represents any file in a cmake project that has been added
 * to the project.
 *
 * Contains the required information to compile it properly
 */
struct CMakeFile
{
    KDevelop::Path::List includes;
    QHash<QString, QString> defines;
};

struct CMakeProjectData
{
    CMakeProperties properties;
    CacheValues cache;
    QHash<KDevelop::Path, CMakeFile> files;
    
    void clear() { properties.clear(); cache.clear(); files.clear(); }
};

#endif
