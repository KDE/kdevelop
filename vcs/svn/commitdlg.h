//stolen from CVS part
#ifndef _COMMITDIALOG_H_
#define _COMMITDIALOG_H_

#include <qdialog.h>
#include <qmultilineedit.h>


class CommitDialog : public QDialog
{
	public:
		CommitDialog();
		QString logMessage() { return edit->text(); }
		void setLog(QString msg) { edit->setText(msg); }
		void append(QString msg) { edit->setText(edit->text()+"\n"+msg); }
		void clear() { edit->setText(""); }

	private:
		QMultiLineEdit *edit;
};

#endif
/* vim: set ai ts=8 sw=8 : */
