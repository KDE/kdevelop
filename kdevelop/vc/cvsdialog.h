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
    CvsDialog( const char *command, const char *text);
    ~CvsDialog();

    virtual void show();
    
protected slots:
    void childExited();
    void receivedOutput(KProcess *proc, char *buffer, int buflen);
    void buttonPressed();
    
private:
    void processOutput();
    void finish();

    bool running;
    QString cmd;
    QMultiLineEdit *resultbox;
    QPushButton *button;
    KProcess *childproc;
    QString buf;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
