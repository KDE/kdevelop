#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <QtCore/QObject>
#include <plasma/view.h>

class IDashboardPlasmoidFactory;
class IDashboardWidgetFactory;
class AppletSelector;
class DashboardCorona;

namespace Plasma {
	class Corona;
}

class dashboard : public Plasma::View
{
    Q_OBJECT
    public:
        dashboard(DashboardCorona* corona, QWidget* parent=0);
        virtual ~dashboard();
        
        virtual void resizeEvent(QResizeEvent* event);
        
    private slots:
        void init();
        void updateView();
        void updateConfigurationMode(bool);
        void addApplet(const QString& name);
        void addApplet(IDashboardPlasmoidFactory* w);
        void addApplet(IDashboardWidgetFactory* w);
        
    private:
        DashboardCorona* corona;
        AppletSelector* m_selector;
};

#endif // dashboard_H
