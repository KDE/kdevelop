#include <qlayout.h>
#include <qlabel.h>
#include <kapp.h>
#include <kbuttonbox.h>
#include <klocale.h>

#include "commitdialog.h"
//#include "commitdialog.moc"


CommitDialog::CommitDialog()
    : QDialog(0, "", true)
{
    setCaption( i18n("CVS Commit") );

    QBoxLayout *layout = new QVBoxLayout(this, 10);
    
    QLabel *messagelabel = new QLabel(i18n("Log message:"), this);
    messagelabel->setMinimumSize(messagelabel->sizeHint());
    layout->addWidget(messagelabel, 0);
    
    edit = new QMultiLineEdit(this);
    QFontMetrics fm(edit->fontMetrics());
    edit->setMinimumSize(fm.width("0")*40, fm.lineSpacing()*3);
    layout->addWidget(edit, 10);
    
    KButtonBox *buttonbox = new KButtonBox(this);
    buttonbox->addStretch();
    QPushButton *ok = buttonbox->addButton(i18n("Ok"));
    QPushButton *cancel = buttonbox->addButton(i18n("Cancel"));
    connect( ok, SIGNAL(clicked()), SLOT(accept()) );
    connect( cancel, SIGNAL(clicked()), SLOT(reject()) );
    ok->setDefault(true);
    buttonbox->layout();
    layout->addWidget(buttonbox, 0);

    layout->activate();
    adjustSize();
}

// Local Variables:
// c-basic-offset: 4
// End:

    
