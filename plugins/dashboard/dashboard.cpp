#include "dashboard.h"
#include <plasma/corona.h>
#include <QTimer>
#include "dashboardcorona.h"

using namespace Plasma;

dashboard::dashboard(DashboardCorona* corona, QWidget* parent)
	: View(corona->containments().first(), parent), corona(corona)
{
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	
	connect(this, SIGNAL(sceneRectAboutToChange()), this, SLOT(updateView()));
	connect(containment(), SIGNAL(toolBoxVisibilityChanged(bool)), this, SLOT(updateConfigurationMode(bool)));
	
	QTimer::singleShot(0, this, SLOT(init()));
}

dashboard::~dashboard()
{}

void dashboard::init()
{
	QVariantList args;
	for(int i=0; i<5; ++i) {
		Applet* applet = Applet::load("clock", 0, args);
		
		corona->containments().first()->addApplet(applet);
	}
	setScene(corona);
	updateView();
}

void dashboard::updateView()
{
	Containment* c=corona->containments().first();
	c->resize(size());
	fitInView(c);
}

void dashboard::resizeEvent(QResizeEvent* event)
{
// 	QGraphicsView::resizeEvent(event);
	updateView();
}

#include "dashboard.moc"
