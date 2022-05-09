#ifndef CONFIG_KDEVPLATFORM_H
#define CONFIG_KDEVPLATFORM_H

#include "kdevelop_version.h"

#define KDEV_ITEMREPOSITORY_VERSION (KDEVELOP_VERSION_MAJOR << 24) \
                                  + (@KDEVELOP_OWN_MINOR_VERSION@ << 16) \
                                  + @KDEV_ITEMREPOSITORY_INCREMENT@

#endif
