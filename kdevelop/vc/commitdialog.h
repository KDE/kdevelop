#ifndef _COMMITDIALOG_H_
#define _COMMITDIALOG_H_

#include <qdialog.h>
#include <qmultilinedit.h>


class CommitDialog : public QDialog
{
  Q_OBJECT

public:
    CommitDialog();
    QString logMessage()
        { return edit->text(); }

private:
    QMultiLineEdit *edit;
};

#endif
