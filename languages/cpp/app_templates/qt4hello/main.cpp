%{CPP_TEMPLATE}

#include <QCoreApplication>
#include "%{APPNAMELC}.h"

int main(int argc, char *argv[])
{
      QCoreApplication app(argc, argv);
      return app.exec();
}

