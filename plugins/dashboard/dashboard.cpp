#include "dashboard.h"
#include <plasma/corona.h>
#include <QTimer>
#include "dashboardcorona.h"
#include "appletselector.h"

using namespace Plasma;

dashboard::dashboard(DashboardCorona* corona, QWidget* parent)
	: View(0, parent), corona(corona), m_selector(0)
{
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	
	QTimer::singleShot(0, this, SLOT(init()));
}

dashboard::~dashboard()
{
	corona->saveLayout(QString());
}

void dashboard::init()
{
	updateView();
	connect(this, SIGNAL(sceneRectAboutToChange()), this, SLOT(updateView()));
	
	Containment* c=corona->containments().first();
	setContainment(c);
	connect(containment(), SIGNAL(toolBoxVisibilityChanged(bool)), this, SLOT(updateConfigurationMode(bool)));
	
	setScene(corona);
}

void dashboard::updateView()
{
	if(!corona->containments().isEmpty()) {
		Containment* c=corona->containments().first();
		
		if (c->size().toSize() != size()) {
// 			c->setMaximumSize(size());
// 			c->setMinimumSize(size());
// 			c->resize(size());
			
			c->resize(size());
			fitInView(c);
		}
	}
}

void dashboard::resizeEvent(QResizeEvent* event)
{
// 	QGraphicsView::resizeEvent(event);
	updateView();
}

void dashboard::updateConfigurationMode(bool mode)
{
	if(mode) { //TODO: Wrong place to put it, can't figure out the correct one yet
		if(!m_selector) {
			m_selector=new AppletSelector(this);
			connect(m_selector, SIGNAL(addApplet(QString)), SLOT(addApplet(QString)));
		}
		
		m_selector->show();
	}
	updateView();
}

void dashboard::addApplet(const QString& name)
{
	Applet* app=containment()->addApplet(name);
	Q_ASSERT(app);
}

#include "dashboard.moc"
