#include "dashboard.h"
#include <plasma/corona.h>
#include "dashboardcorona.h"
#include "appletselector.h"
#include <interfaces/idashboardfactory.h>
#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>

using namespace Plasma;

dashboard::dashboard(KDevelop::IProject* project, DashboardCorona* corona, QWidget* parent)
    : View(0, parent), corona(corona), m_selector(0), m_project(project)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
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
    connect(containment(), SIGNAL(showAddWidgetsInterface(QPointF)), this, SLOT(showAppletsSwitcher()));
    
    setScene(corona);
}

void dashboard::updateView()
{
    if(!corona->containments().isEmpty()) {
        Containment* c=corona->containments().first();
        
        if (c->size().toSize() != size()) {
//             c->setMaximumSize(size());
//             c->setMinimumSize(size());
//             c->resize(size());
            
            c->resize(size());
            ensureVisible(c);
        }
    }
}

void dashboard::resizeEvent(QResizeEvent* event)
{
// 	QGraphicsView::resizeEvent(event);
    updateView();
}

void dashboard::showAppletsSwitcher()
{
    if(!m_selector) {
        m_selector=new AppletSelector(this);
        connect(m_selector, SIGNAL(addApplet(QString)), SLOT(addApplet(QString)));
        connect(m_selector, SIGNAL(addApplet(IDashboardPlasmoidFactory*)), SLOT(addApplet(IDashboardPlasmoidFactory*)));
        connect(m_selector, SIGNAL(addApplet(IDashboardWidgetFactory*)), SLOT(addApplet(IDashboardWidgetFactory*)));
    }
    
    m_selector->show();
}

void dashboard::addApplet(const QString& name)
{
    Applet* app=containment()->addApplet(name);
    Q_ASSERT(app);
}

void dashboard::addApplet(IDashboardPlasmoidFactory* fact)
{
    fact->setProject(m_project);
    Applet* applet=fact->plasmaApplet(QString());
    containment()->addApplet(applet);
}

void dashboard::addApplet(IDashboardWidgetFactory* fact)
{
    fact->setProject(m_project);
    QWidget* w=fact->widget();
    w->setAttribute(Qt::WA_NoSystemBackground);
    
    Applet* a = new Applet(0, "clock");
    a->setBackgroundHints(Plasma::Applet::StandardBackground);
    QGraphicsLinearLayout* l=new QGraphicsLinearLayout(Qt::Horizontal);
    QGraphicsProxyWidget* proxy=new QGraphicsProxyWidget(a);
    proxy->setWidget(w);
    l->addItem(proxy);
    a->setLayout(l);
    containment()->addApplet(a);
}

#include "dashboard.moc"
