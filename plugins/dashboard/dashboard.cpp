#include "dashboard.h"
#include <plasma/corona.h>
#include <QTimer>
#include "dashboardcorona.h"

using namespace Plasma;

dashboard::dashboard(DashboardCorona* corona, QWidget* parent)
	: View(0, parent), corona(corona)
{
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	
	connect(this, SIGNAL(sceneRectAboutToChange()), this, SLOT(updateView()));
	connect(containment(), SIGNAL(toolBoxVisibilityChanged(bool)), this, SLOT(updateConfigurationMode(bool)));
	
	QTimer::singleShot(0, this, SLOT(init()));
}

dashboard::~dashboard()
{
    corona->saveLayout(QString());
}

void dashboard::init()
{
	setScene(corona);
	updateView();
}

void dashboard::updateView()
{
    if(!corona->containments().isEmpty()) {
        Containment* c=corona->containments().first();
        if(c->size()==size())
            return;
        
        c->resize(size());
        fitInView(c);
        
//         c->setMaximumSize(size());
//         c->setMinimumSize(size());
//         c->resize(size());
    }
}

void dashboard::resizeEvent(QResizeEvent* event)
{
// 	QGraphicsView::resizeEvent(event);
	updateView();
}

void dashboard::updateConfigurationMode ( bool )
{
    updateView();
}


#include "dashboard.moc"
