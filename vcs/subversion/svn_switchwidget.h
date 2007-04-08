#ifndef SVN_SWITCHWIDGET_H
#define SVN_SWITCHWIDGET_H

#include "svn_switchdlgbase.h"

namespace SvnGlobal
{
	class SvnInfoHolder;
}

class SvnSwitchDlg : public SvnSwitchDlgBase
{
	Q_OBJECT
public:
	SvnSwitchDlg( const SvnGlobal::SvnInfoHolder *holder,
				  const QString &wcPath, QWidget *parent = NULL );
	virtual ~SvnSwitchDlg();

	const QString currentUrl();
	const QString destUrl();
	bool recursive();
	bool switchOnly();
	bool relocation();
private slots:
	void resetCurrentRepositoryUrlEdit();
	
private:
	const SvnGlobal::SvnInfoHolder *m_info;
};

#endif
