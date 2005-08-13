
#ifndef KDEVCPPSUPPORTIFACE_H
#define KDEVCPPSUPPORTIFACE_H

#include <qobject.h>
#include <dcopobject.h>

class CppSupportPart;

class KDevCppSupportIface : public QObject, public DCOPObject
{
	Q_OBJECT
	K_DCOP
public:
	KDevCppSupportIface( CppSupportPart* cppSupport );
	~KDevCppSupportIface();

k_dcop:
	void addClass();
	void parseProject();

private:
	CppSupportPart* m_cppSupport;
};

#endif 
// kate: indent-mode csands; tab-width 4;
