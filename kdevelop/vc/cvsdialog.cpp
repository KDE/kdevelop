
#include <qlayout.h>
#include <qlabel.h>
#include <qmultilinedit.h>
#include <qlistbox.h>
#include <qmessagebox.h>
#include <kapp.h>
#include <kprocess.h>
#include <kbuttonbox.h>
#include <klocale.h>
#include "cvsdialog.h"

// TODO: Only show dialog if cvs client doesn't exit immediately.
// For local repositories cvs is really fast!

CvsDialog::CvsDialog(const char *text)
    : QDialog(0, "", true), childproc(0)
{
    setCaption( i18n("CVS") );

    QBoxLayout *layout = new QVBoxLayout(this, 10);

    QLabel *textlabel = new QLabel(text, this);
    textlabel->setMinimumWidth(textlabel->sizeHint().width());
    textlabel->setFixedHeight(textlabel->sizeHint().height());
    layout->addWidget(textlabel, 0);
    layout->addSpacing(4);

    resultbox = new QMultiLineEdit(this);
    resultbox->setReadOnly(true);
    QFontMetrics rb_fm(resultbox->fontMetrics());
    resultbox->setMinimumSize(rb_fm.width("0")*75,
			      rb_fm.lineSpacing()*8);
    layout->addWidget(resultbox, 5);
    
    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    frame->setMinimumHeight(frame->sizeHint().height());
    layout->addWidget(frame, 0);

    KButtonBox *buttonbox = new KButtonBox(this);
    layout->addWidget(buttonbox, 0);
    buttonbox->addStretch();
    button = buttonbox->addButton(i18n("Cancel"));
    connect( button, SIGNAL(clicked()), SLOT(buttonPressed()) );
    buttonbox->addStretch();
    buttonbox->layout();

    layout->activate();
    resize(sizeHint());
}


CvsDialog::~CvsDialog()
{
    if (childproc)
	delete childproc;
}


void CvsDialog::processOutput()
{
    int pos;
    while ( (pos = buf.find('\n')) != -1)
	{
	    QString item = buf.left(pos);
	    if (!item.isEmpty())
		resultbox->insertLine(item);
	    buf = buf.right(buf.length()-pos-1);
	}
    resultbox->setCursorPosition(resultbox->numLines(), 0, false);
}


void CvsDialog::finish()
{
    button->setText(i18n("OK"));
    button->setDefault(true);
    
    buf += '\n';
    processOutput();
    if (childproc)
	delete childproc;
    childproc = 0;
}


void CvsDialog::buttonPressed()
{
    if (childproc)
        finish();
    else
        accept();
}


void CvsDialog::childExited()
{
    finish();
}


void CvsDialog::receivedOutput(KProcess *proc, char *buffer, int buflen)
{
    buf += QCString(buffer, buflen+1);
    processOutput();
}


void CvsDialog::startCommand(const char *cmd)
{
    resultbox->append(cmd);
    childproc = new KShellProcess();
    *childproc << cmd;

    connect( childproc, SIGNAL(processExited(KProcess *)),
	     SLOT(childExited()) );
    connect( childproc, SIGNAL(receivedStdout(KProcess *, char *, int)),
	     SLOT(receivedOutput(KProcess *, char *, int)) );
    connect( childproc, SIGNAL(receivedStderr(KProcess *, char *, int)),
	     SLOT(receivedOutput(KProcess *, char *, int)) );

    childproc->start(KProcess::NotifyOnExit,
		     KProcess::Communication(KProcess::Stdout|KProcess::Stderr));
}


// Local Variables:
// c-basic-offset: 4
// End:

    

