%{H_TEMPLATE}

#ifndef _%{APPNAMEUC}KMDIVIEW_H
#define _%{APPNAMEUC}KMDIVIEW_H

#include <kparts/part.h>
#include <kmdichildview.h>
#include <kmdichildfrm.h>

/**
 * This class serves as the %{APPNAMELC}kmdi view.
 *
 * @short main view class
 */
class %{APPNAMELC}kmdiView : public KMdiChildView 
{
	Q_OBJECT
	public:
		%{APPNAMELC}kmdiView( QWidget *parentWidget=0L, const char *name=0L );
		virtual ~%{APPNAMELC}kmdiView();

		KParts::ReadWritePart *part() { return m_part; }

	private:
		KParts::ReadWritePart *m_part;
};

#endif // _%{APPNAMEUC}KMDIVIEW_H

