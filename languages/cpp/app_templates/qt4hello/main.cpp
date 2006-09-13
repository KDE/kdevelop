%{CPP_TEMPLATE}

#include <QCoreApplication>

int main(int argc, char *argv[])
{
      QCoreApplication app(argc, argv);
      qDebug("Hello from Qt 4!");
      return 0;
}

