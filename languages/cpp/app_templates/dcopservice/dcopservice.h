
#ifndef _%{APPNAME}_H
#define _%{APPNAME}_H

#include <qstringlist.h>
#include <qstring.h>
#include <dcopobject.h>

class %{APPNAME} :  public DCOPObject
{
	K_DCOP

	private:
		QStringList m_List;

	public:
		%{APPNAME}();

		~%{APPNAME}();

	k_dcop:
		QString string(int);

		QStringList list();

		void add(QString);

		bool remove(QString);

		bool exit();


};
#endif
