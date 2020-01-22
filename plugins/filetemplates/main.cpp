#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include "templateclassassistant.h"
#include <QApplication>
using namespace KDevelop;

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);

    auto* assistant = new TemplateClassAssistant(QApplication::activeWindow(), QUrl::fromLocalFile(QStringLiteral("/tmp/")));
    assistant->setAttribute(Qt::WA_DeleteOnClose);
    assistant->show();
    return app.exec();
}
