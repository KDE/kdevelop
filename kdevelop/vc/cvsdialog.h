#ifndef _CVSDIALOG_H_
#define _CVSDIALOG_H_

#include <qdialog.h>


class QListBox;
class QMultiLineEdit;
class KProcess;


class CvsDialog : public QDialog
{
    Q_OBJECT

public:
    CvsDialog(const char *text);
    ~CvsDialog();

    void startCommand(const char *cmd);
    
protected slots:
    void childExited();
    void receivedOutput(KProcess *proc, char *buffer, int buflen);
    void buttonPressed();
    
private:
    void processOutput();
    void finish();

    QMultiLineEdit *resultbox;
    QPushButton *button;
    KProcess *childproc;
    QString buf;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
