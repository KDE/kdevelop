#ifndef ASKTARGETNAME_H
#define ASKTARGETNAME_H

#include <QLineEdit>
#include <QLabel>
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>

class AskTargetName : public QDialog
{
    public:
        AskTargetName(QWidget* parent) : QDialog(parent)
        {
            QDialogButtonBox * buttonBox;
            QVBoxLayout *items=new QVBoxLayout(this);
            items->addWidget(new QLabel(i18n("Enter a name for the target"), this));
            items->addWidget(edit=new QLineEdit(this));
    //         items->addItem(new QSpacerItem());
            items->addWidget(buttonBox=new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok, Qt::Horizontal, this));
            
            connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
            connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
        }
        
        QString name() const { return edit->text(); }
    private:
        QLineEdit *edit;
};

#endif
