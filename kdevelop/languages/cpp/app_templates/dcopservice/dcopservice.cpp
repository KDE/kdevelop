%{CPP_TEMPLATE}

#include "%{APPNAMELC}.h"
#include <kdebug.h>
#include <kapplication.h>

%{APPNAME}::%{APPNAME}() : DCOPObject("serviceInterface")
{
	kdDebug() << "Starting new service... " << endl;
	m_List = QStringList();
}

%{APPNAME}::~%{APPNAME}()
{
	kdDebug() << "Going away... " << endl;
}

QString %{APPNAME}::string(int idx)
{
	return *m_List.at(idx);
}

QStringList %{APPNAME}::list()
{
	return m_List;
}

void %{APPNAME}::add(QString arg)
{
	kdDebug() << "Adding " << arg << " to the list" << endl;
	m_List << arg;
}

bool %{APPNAME}::remove(QString arg)
{
	QStringList::Iterator it = m_List.find(arg);
	if (it != m_List.end())
	{
		m_List.remove(it);
	}
	else
		return false;
	return true;
}

bool %{APPNAME}::exit()
{
	kapp->quit();
}
