#ifndef CONFIG_KDEVPLATFORM_H
#define CONFIG_KDEVPLATFORM_H

#include "kdevelop_version.h"

#define KDEV_ITEMREPOSITORY_VERSION (KDEVELOP_VERSION_MAJOR << 24) \
                                  + (KDEVELOP_VERSION_MINOR << 16) \
                                  + (KDEVELOP_VERSION_PATCH << 8) \
                                  + @KDEV_ITEMREPOSITORY_INCREMENT@

#endif
