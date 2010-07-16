#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <QtCore/QObject>
#include <plasma/view.h>

class IDashboardPlasmoidFactory;
class IDashboardWidgetFactory;
class AppletSelector;
class DashboardCorona;

namespace KDevelop {
    class IProject;
}
namespace Plasma {
	class Corona;
}

class dashboard : public Plasma::View
{
    Q_OBJECT
    public:
        dashboard(KDevelop::IProject* project, DashboardCorona* corona, QWidget* parent=0);
        virtual ~dashboard();
        
        virtual void resizeEvent(QResizeEvent* event);
        
    private slots:
        void updateView();
        void showAppletsSwitcher();
        void addApplet(const QString& name);
        void addApplet(IDashboardPlasmoidFactory* w);
        void addApplet(IDashboardWidgetFactory* w);
        
    private:
        DashboardCorona* corona;
        AppletSelector* m_selector;
        KDevelop::IProject* m_project;
};

#endif // dashboard_H
