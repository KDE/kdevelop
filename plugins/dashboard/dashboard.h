#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <QtCore/QObject>
#include <plasma/view.h>

namespace Plasma {
	class Corona;
}
class DashboardCorona;

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
        void updateConfigurationMode ( bool );
		
	private:
		DashboardCorona* corona;
};

#endif // dashboard_H
