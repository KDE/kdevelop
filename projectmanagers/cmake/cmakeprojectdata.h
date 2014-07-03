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
struct CMakeFiles
{
    KDevelop::Path::List includes;
    QHash<QString, QString> defines;
};

struct CMakeProjectData
{
    CMakeProperties properties;
    CacheValues cache;
    QHash<KDevelop::Path, CMakeFiles> files;
    
    void clear() { properties.clear(); cache.clear(); }
};

#endif
