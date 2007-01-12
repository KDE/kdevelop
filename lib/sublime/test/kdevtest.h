#ifndef KDEVTEST
#define KDEVTEST

#include <kaboutdata.h>
#include <kcmdlineargs.h>

#define KDEVTEST_MAIN(TestObject) \
int main(int argc, char *argv[]) \
{ \
    static const char description[] = "Sublime Library Test"; \
    KAboutData aboutData("test", "Test", \
                         "1.0", description, KAboutData::License_LGPL, \
                         "(c) 2007, KDevelop Developers", "", "http://www.kdevelop.org" ); \
 \
    KCmdLineArgs::init(argc, argv, &aboutData); \
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs(); \
    KApplication app; \
 \
    TestObject tc; \
    return QTest::qExec(&tc, argc, argv); \
}

#endif
