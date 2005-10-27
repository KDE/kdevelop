%{CPP_TEMPLATE}

#include <QApplication>
#include "%{APPNAMELC}.h"

int main(int argc, char *argv[])
{
      Q_INIT_RESOURCE(application);
      QApplication app(argc, argv);
      %{APPNAME} * mw = new %{APPNAME}();
      mw->show();
      return app.exec();
}

