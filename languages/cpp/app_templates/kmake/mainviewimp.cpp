
#include "mainviewimp.h"
#include <qlabel.h>
#include <klocale.h>

MainViewImp::MainViewImp(QWidget* parent, const char* name, Qt::WFlags fl)
: MainView(parent,name,fl)
{
}

MainViewImp::~MainViewImp()
{
}

void MainViewImp::slotHelloButtonClicked()
{
	helloLabel->setText(i18n("Hello there."));
}





