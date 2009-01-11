#ifndef SESSIONCONFIGSKELETON_H
#define SESSIONCONFIGSKELETON_H

#include <kconfigskeleton.h>

#include "../core.h"
#include "../session.h"

namespace KDevelop
{

class SessionConfigSkeleton : public KConfigSkeleton
{
public:
    SessionConfigSkeleton( const QString& )
        : KConfigSkeleton( Core::self()->activeSession()->config() )
    {
    }
};

}

#endif
