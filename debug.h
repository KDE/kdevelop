#ifndef DEBUG_H
#define DEBUG_H

#include <KDebug>

static int debugArea()
{
    static int s_area = KDebug::registerArea("kdevelop (Clang support)");
    return s_area;
}
#define debug() kDebug(debugArea())

#endif // DEBUG_H