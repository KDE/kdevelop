#ifndef QMAKECONFIG_H
#define QMAKECONFIG_H

class QMakeConfig
{
public:
    static const char *CONFIG_GROUP;
        
    static const char 
        *QMAKE_BINARY,
        *BUILD_FOLDER,
        *INSTALL_PREFIX,
        *EXTRA_ARGUMENTS,
        *BUILD_TYPE;
};

#endif
