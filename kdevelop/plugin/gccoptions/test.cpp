#include <qlayout.h>
#include <qlabel.h>
#include <qdialog.h>
#include <qstringlist.h>
#include <kapp.h>
#include <kcmdlineargs.h>
#include <klibloader.h>

#include "kdevcompileroptions.h"

extern "C" {
    void *init_libkdevgccoptions();
};


int main(int argc, char *argv[])
{
    KCmdLineArgs::init(argc, argv, "test", "blubb", "0.0");
    KApplication app;
    QDialog *dlg = new QDialog();
    QStringList args;
    args << "gcc";

    QBoxLayout *layout = new QVBoxLayout(dlg);
    layout->setAutoAdd(true);
    //    QLabel *blubb = new  QLabel("I'm a label", box);

    KLibFactory *factory = static_cast<KLibFactory*>(init_libkdevgccoptions());
    
    KDevCompilerOptions *c =
        static_cast<KDevCompilerOptions*>(factory->create(dlg, "widget", "KDevCompilerOptions", args));
    c->setFlags("-Wall");

    dlg->show();

    return app.exec();
}
