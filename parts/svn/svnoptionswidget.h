#ifndef _SVNOPTIONSWIDGET_H_
#define _SVNOPTIONSWIDGET_H_

#include <qwidget.h>
#include "svnoptions.h"

class svnPart;

class svnOptionsWidget : public svnoptions
{
	Q_OBJECT

	public:
		svnOptionsWidget(svnPart *widget, QWidget *parent, const char *name=0);
		~svnOptionsWidget();

		public slots:
			void accept();

	private:
		void readConfig();
		void storeConfig();

		svnPart *m_part;
};

#endif

/* vim: set ai ts=8 sw=8 : */
