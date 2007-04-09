#ifndef SVN_COPYWIDGET_H
#define SVN_COPYWIDGET_H

#include "svn_copydlgwidget.h"

namespace SvnGlobal
{
	class SvnInfoHolder;
}
class KURL;

class SvnCopyDialog : public SvnCopyDialogBase
{
	Q_OBJECT
public:
	SvnCopyDialog( const QString &reqPath, SvnGlobal::SvnInfoHolder *holder, QWidget *parent );
	~SvnCopyDialog();

	KURL sourceUrl();
	int revision();
	QString revKind();
	KURL destUrl();

public slots:
	void setSourceAsUrl();
	void setSourceAsLocalPath();
	
private:
	SvnGlobal::SvnInfoHolder *m_holder;
};

#endif
