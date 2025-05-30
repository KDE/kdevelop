#include "filetemplateprovider.h"
#include "templateclassassistant.h"

#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <QApplication>
#include <QUrl>

using namespace KDevelop;

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);

    FileTemplateProvider templateProvider;
    auto* const assistant = new TemplateClassAssistant(templateProvider, QUrl::fromLocalFile(QStringLiteral("/tmp/")),
                                                       QApplication::activeWindow());
    assistant->setAttribute(Qt::WA_DeleteOnClose);
    assistant->show();
    return app.exec();
}
