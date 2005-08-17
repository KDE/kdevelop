//Added by qt3to4:
#include <QShowEvent>
%{H_TEMPLATE}

#ifndef KMDIKONSOLE_H
#define KMDIKONSOLE_H

#include <q3vbox.h>

class kmdikonsole : public Q3VBox
{
	Q_OBJECT

	public:
		kmdikonsole( QWidget* parent, const char* name=0);
		~kmdikonsole();

	public slots:
		void setDirectory(const QString & dir);
		void activate();

	private slots:
		void slotDestroyed();
		void respawn();

	protected:
		void showEvent(QShowEvent *ev);

	private:
		KParts::ReadOnlyPart *m_part;
		bool m_haskonsole;
};

#endif // KMDIKONSOLE_H
