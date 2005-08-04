/* This file is part of the KDE project
   Copyright (C) 2000-2001 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright (C) 2003 Alexander Dymo <cloudtemple@mksat.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include <klocale.h>
#include <kdebug.h>
#include <kurlrequester.h>
#include <klineedit.h>
#include <kdialogbase.h>
#include <kdeversion.h>

#include <qapplication.h>
#include <qtooltip.h>
#include <q3header.h>
#include <qstringlist.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qregexp.h>
#include <qspinbox.h>
//Added by qt3to4:
#include <QEvent>
#include <QHBoxLayout>
#include <QBoxLayout>
#include <QVBoxLayout>

// only for KDE < 3.1
#if KDE_VERSION <= 305
#include "../compat/kdeveditlistbox.h"
using namespace KDevCompat;
#endif

#include "flagboxes.h"

// partial copy of Qt-3.1 for back-compatibility to KDE-3.0
QString QRegExp_escape( const QString& str )
{
    static const char meta[] = "$()*+.?[\\]^{|}";
    QString quoted = str;
    int i = 0;

    while ( i < (int) quoted.length() ) {
	if ( strchr(meta, quoted[i].latin1()) != 0 )
	    quoted.insert( i++, "\\" );
	i++;
    }
    return quoted;
}


class FlagListToolTip : public QToolTip
{
public:
    FlagListToolTip(QWidget *parent);
protected:
    void maybeTip(const QPoint &p);
};


FlagListToolTip::FlagListToolTip(QWidget *parent)
    : QToolTip(parent)
{}


void FlagListToolTip::maybeTip(const QPoint &pos)
{
    FlagListBox *listbox = static_cast<FlagListBox*>(parentWidget());
    Q3ListViewItem *item = listbox->itemAt(pos);
    FlagListItem *flitem = static_cast<FlagListItem*>(item);

    if (item)
        tip(listbox->itemRect(item), flitem->desc);
}


FlagListItem::FlagListItem(FlagListBox *parent, const QString &flagstr,
                           const QString &description)
    : Q3CheckListItem(parent, flagstr, Q3CheckListItem::CheckBox),
      flag(flagstr), desc(description)
{}


FlagListItem::FlagListItem(FlagListBox *parent, const QString &flagstr,
                           const QString &description, const QString &offstr)
    : Q3CheckListItem(parent, flagstr, Q3CheckListItem::CheckBox),
      flag(flagstr), off(offstr), desc(description)
{}


FlagListBox::FlagListBox(QWidget *parent, const char *name)
    : Q3ListView(parent, name)
{
    setResizeMode(LastColumn);
    header()->hide();
    addColumn(i18n("Flags"));
    (void) new FlagListToolTip(this);
}


void FlagListBox::readFlags(QStringList *list)
{
    Q3ListViewItem *item = firstChild();
    for (; item; item = item->nextSibling()) {
        FlagListItem *flitem = static_cast<FlagListItem*>(item);
        QStringList::Iterator sli = list->find(flitem->flag);
        if (sli != list->end()) {
            flitem->setOn(true);
            list->remove(sli);
        }
        sli = list->find(flitem->off);
        if (sli != list->end()) {
            flitem->setOn(false);
            list->remove(sli);
        }
    }
}


void FlagListBox::writeFlags(QStringList *list)
{
    Q3ListViewItem *item = firstChild();
    for (; item; item = item->nextSibling()) {
        FlagListItem *flitem = static_cast<FlagListItem*>(item);
        if (flitem->isOn())
            (*list) << flitem->flag;
    }
}


FlagCheckBox::FlagCheckBox(QWidget *parent, FlagCheckBoxController *controller,
                           const QString &flagstr, const QString &description)
    : QCheckBox(description, parent), flag(flagstr), includeOff(false), useDef(false), defSet(false)
{
    QToolTip::add(this, flagstr);
    controller->addCheckBox(this);
}


FlagCheckBox::FlagCheckBox(QWidget *parent, FlagCheckBoxController *controller,
                           const QString &flagstr, const QString &description,
                           const QString &offstr)
    : QCheckBox(description, parent), flag(flagstr), off(offstr), includeOff(false), useDef(false), defSet(false)
{
    QToolTip::add(this, flagstr);
    controller->addCheckBox(this);
}

FlagCheckBox::FlagCheckBox(QWidget *parent, FlagCheckBoxController *controller,
                           const QString &flagstr, const QString &description,
                           const QString &offstr, const QString &defstr)
    : QCheckBox(description, parent), flag(flagstr), off(offstr), def(defstr), includeOff(false), useDef(true), defSet(false)
{
    QToolTip::add(this, flagstr);
    controller->addCheckBox(this);
}

FlagCheckBoxController::FlagCheckBoxController(QStringList multiKeys)
    :m_multiKeys(multiKeys)
{
}


void FlagCheckBoxController::addCheckBox(FlagCheckBox *item)
{
    cblist.append(item);
}


void FlagCheckBoxController::readFlags(QStringList *list)
{
    //handle keys like -vxyz -> transform they into -vx -vy -vz
    //very "effective" algo :(
/*    QStringList addons;
    for (QStringList::Iterator mk = m_multiKeys.begin(); mk != m_multiKeys.end(); ++ mk)
    {
        kdDebug() << "multikey " << *mk << endl;
        for (QStringList::Iterator sli = list->begin(); sli != list->end(); ++sli)
        {
            QString key = *sli;
            kdDebug() << "current key: " << key << endl;
            if ( (key.length() > 3) && (key.startsWith(*mk)) )
            {
                list->remove(sli);
                key = key.remove(*mk);
                kdDebug() << "refined key " << key << endl;
                for (int i = 0; i < key.length(); ++i)
                {
                    kdDebug() << "adding key " << *mk + key[i] << endl;
                    addons << *mk + key[i];
                }
            }
        }
    }
    kdDebug() << "good" << endl;
    *list += addons;

    for (QStringList::Iterator sli = list->begin(); sli != list->end(); ++sli)
    {
        kdDebug() << "KEYS: " << *sli << endl;
    }
*/
    Q3PtrListIterator<FlagCheckBox> it(cblist);
    for (; it.current(); ++it) {
        FlagCheckBox *fitem = it.current();
        QStringList::Iterator sli = list->find(fitem->flag);
        if (sli != list->end()) {
            fitem->setChecked(true);
            fitem->useDef = false;
            list->remove(sli);
        }
        sli = list->find(fitem->off);
        if (sli != list->end()) {
            fitem->setChecked(false);
            fitem->includeOff = true;
            fitem->useDef = false;
            list->remove(sli);
        }
        if (!fitem->def.isEmpty())
            if (fitem->useDef && (fitem->def == fitem->flag))
            {
                fitem->setChecked(true);
                fitem->defSet = true;
            }
        else
            fitem->useDef = false;
    }
}


void FlagCheckBoxController::writeFlags(QStringList *list)
{
    Q3PtrListIterator<FlagCheckBox> it(cblist);
    for (; it.current(); ++it) {
        FlagCheckBox *fitem = it.current();
        if (fitem->isChecked() && (!fitem->useDef))
        {
            (*list) << fitem->flag;
        }
        else if ((!fitem->off.isEmpty()) && fitem->includeOff)
            (*list) << fitem->off;
        else if ((fitem->def == fitem->flag) && (!fitem->isChecked()))
            (*list) << fitem->off;
        else if ((fitem->def == fitem->off) && (fitem->isChecked()))
            (*list) << fitem->flag;
    }
}

 FlagPathEditController::FlagPathEditController( )
{
}

 FlagPathEditController::~ FlagPathEditController( )
{
}

void FlagPathEditController::readFlags( QStringList * list )
{
//    kdDebug() << "read path flags" << endl;
    Q3PtrListIterator<FlagPathEdit> it(plist);
    for (; it.current(); ++it) {
        FlagPathEdit *peitem = it.current();

        QStringList::Iterator sli = list->begin();
        while ( sli != list->end() )
        {
  //          kdDebug() << "option: " << (*sli) << " flag is: " << peitem->flag << endl;
            if ((*sli).startsWith(peitem->flag))
            {
//                kdDebug() << "Processing.." << endl;
                peitem->setText((*sli).replace(QRegExp(QRegExp_escape(peitem->flag)),""));
                sli = list->remove(sli);
                continue;
            }
             ++sli;
        }
/*        QStringList::Iterator sli = list->find(peitem->flag);
        if (sli != list->end()) {
            peitem->setText((*sli).remove(peitem->flag));
            list->remove(sli);
        }*/
    }
}

void FlagPathEditController::writeFlags( QStringList * list )
{
    Q3PtrListIterator<FlagPathEdit> it(plist);
    for (; it.current(); ++it) {
        FlagPathEdit *pitem = it.current();
        if (!pitem->isEmpty())
            (*list) << pitem->flag + pitem->text();
    }
}

void FlagPathEditController::addPathEdit( FlagPathEdit * item )
{
    plist.append(item);
}

FlagPathEdit::FlagPathEdit( QWidget * parent, QString pathDelimiter,
    FlagPathEditController * controller, const QString & flagstr, const QString & description,
    KFile::Mode mode )
    : QWidget(parent), delimiter(pathDelimiter), flag(flagstr), m_description(description)
{
    QBoxLayout *topLayout = new QVBoxLayout(this, 0, 1);
    topLayout->addWidget(new QLabel(description, this));
    QBoxLayout *layout = new QHBoxLayout(topLayout, KDialog::spacingHint());

    if (delimiter.isEmpty())
    {
        url = new KURLRequester(this);
        url->setMode(mode);
        layout->addWidget(url);
    }
    else
    {
        edit = new KLineEdit(this);
        layout->addWidget(edit);
        details = new QPushButton("...", this);
        details->setMaximumWidth(30);
        connect(details, SIGNAL(clicked()), this, SLOT(showPathDetails()));
        layout->addWidget(details);
    }

    QApplication::sendPostedEvents(this, QEvent::ChildInserted);

    QToolTip::add(this, flagstr);
    controller->addPathEdit(this);
}

void FlagPathEdit::showPathDetails( )
{
    KDialogBase *dia = new KDialogBase(0, "flag_path_edit_dia", true, m_description,
        KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true);

    QBoxLayout *diaLayout = new QVBoxLayout(dia, KDialog::marginHint(), KDialog::spacingHint());
    diaLayout->setAutoAdd(true);

    KURLRequester *req = new KURLRequester( dia );
    req->setMode(KFile::Directory);
    KEditListBox::CustomEditor pCustomEditor;
#if KDE_VERSION > 305
    pCustomEditor = req->customEditor();
#else
    QObjectList* pOL = req->queryList("KLineEdit"); // dirty hack :)
    KLineEdit* pEdit = dynamic_cast<KLineEdit*>(pOL->first());
    assert(pEdit);
    KEditListBox::CustomEditor editor(req, pEdit);
    pCustomEditor = editor;
#endif
    KEditListBox *elb = new KEditListBox( "", pCustomEditor, dia );
    dia->setMainWidget(elb);

    elb->insertStringList(QStringList::split(delimiter, text()));

    if (dia->exec() == QDialog::Accepted)
    {
        setText(elb->items().join(delimiter));
    }

    delete dia;
}

void FlagPathEdit::setText( const QString text )
{
    if (delimiter.isEmpty())
        url->setURL(text);
    else
        edit->setText(text);
}

QString FlagPathEdit::text( )
{
    if (delimiter.isEmpty())
        return url->url();
    else
        return edit->text();
}

bool FlagPathEdit::isEmpty( )
{
    if (delimiter.isEmpty())
        return url->url().isEmpty();
    else
        return edit->text().isEmpty();
}

FlagRadioButton::FlagRadioButton( QWidget * parent, FlagRadioButtonController * controller, const QString & flagstr, const QString & description )
    : QRadioButton(description, parent), flag(flagstr)
{
    QToolTip::add(this, flagstr);
    controller->addRadioButton(this);
}

FlagRadioButtonController::FlagRadioButtonController(QStringList multiKeys)
    :m_multiKeys(multiKeys)
{
}

void FlagRadioButtonController::addRadioButton(FlagRadioButton *item)
{
    cblist.append(item);
}


void FlagRadioButtonController::readFlags(QStringList *list)
{
    //handle keys like -vxyz -> transform they into -vx -vy -vz
    //very "effective" algo :(
/*    QStringList addons;
    for (QStringList::Iterator mk = m_multiKeys.begin(); mk != m_multiKeys.end(); ++ mk)
    {
        kdDebug() << "multikey " << *mk << endl;
        for (QStringList::Iterator sli = list->begin(); sli != list->end(); ++sli)
        {
            QString key = *sli;
            kdDebug() << "current key: " << key << endl;
            if ( (key.length() > 3) && (key.startsWith(*mk)) )
            {
                list->remove(sli);
                key = key.remove(*mk);
                kdDebug() << "refined key " << key << endl;
                for (int i = 0; i < key.length(); ++i)
                {
                    kdDebug() << "adding key " << *mk + key[i] << endl;
                    addons << *mk + key[i];
                }
            }
        }
    }
    kdDebug() << "good" << endl;
    *list += addons;

    for (QStringList::Iterator sli = list->begin(); sli != list->end(); ++sli)
    {
        kdDebug() << "KEYS: " << *sli << endl;
    }
*/
    Q3PtrListIterator<FlagRadioButton> it(cblist);
    for (; it.current(); ++it) {
        FlagRadioButton *fitem = it.current();
        QStringList::Iterator sli = list->find(fitem->flag);
        if (sli != list->end()) {
            fitem->setChecked(true);
            list->remove(sli);
        }
    }
}


void FlagRadioButtonController::writeFlags(QStringList *list)
{
    Q3PtrListIterator<FlagRadioButton> it(cblist);
    for (; it.current(); ++it) {
        FlagRadioButton *fitem = it.current();
        if (fitem->isChecked())
            (*list) << fitem->flag;
    }
}

 FlagEditController::FlagEditController( )
{
}

 FlagEditController::~ FlagEditController( )
{
}

void FlagEditController::readFlags( QStringList * list )
{
    Q3PtrListIterator<FlagListEdit> it(plist);
    for (; it.current(); ++it) {
        FlagListEdit *peitem = it.current();

        QStringList::Iterator sli = list->begin();
        while (sli != list->end())
        {
            if ((*sli).startsWith(peitem->flag))
            {
                peitem->appendText((*sli).replace(QRegExp(QRegExp_escape(peitem->flag)),""));
                sli = list->remove(sli);
                continue;
            }
            ++sli;
        }
    }


    Q3PtrListIterator<FlagSpinEdit> it2(slist);
    for (; it2.current(); ++it2) {
        FlagSpinEdit *sitem = it2.current();

        QStringList::Iterator sli = list->begin();
        while ( sli != list->end() )
        {
            if ((*sli).startsWith(sitem->flag))
            {
                sitem->setText((*sli).replace(QRegExp(QRegExp_escape(sitem->flag)),""));
                sli = list->remove(sli);
                continue;
            }
            ++sli;
        }
    }
}

void FlagEditController::writeFlags( QStringList * list )
{
    Q3PtrListIterator<FlagListEdit> it(plist);
    for (; it.current(); ++it) {
        FlagListEdit *pitem = it.current();
        if (!pitem->isEmpty())
            (*list) += pitem->flags();
    }

    Q3PtrListIterator<FlagSpinEdit> it2(slist);
    for (; it2.current(); ++it2) {
        FlagSpinEdit *sitem = it2.current();
        if (!sitem->isDefault())
            (*list) << sitem->flags();
    }
}

void FlagEditController::addListEdit( FlagListEdit * item )
{
    plist.append(item);
}

void FlagEditController::addSpinBox(FlagSpinEdit *item)
{
    slist.append(item);
}


FlagListEdit::FlagListEdit( QWidget * parent, QString listDelimiter, FlagEditController * controller,
    const QString & flagstr, const QString & description)
    : QWidget(parent), delimiter(listDelimiter), flag(flagstr), m_description(description)
{
    QBoxLayout *topLayout = new QVBoxLayout(this, 0, 1);
    topLayout->addWidget(new QLabel(description, this));
    QBoxLayout *layout = new QHBoxLayout(topLayout, KDialog::spacingHint());

    edit = new KLineEdit(this);
    layout->addWidget(edit);
    if (! listDelimiter.isEmpty())
    {
        details = new QPushButton("...", this);
        details->setMaximumWidth(30);
        connect(details, SIGNAL(clicked()), this, SLOT(showListDetails()));
        layout->addWidget(details);
    }

    QApplication::sendPostedEvents(this, QEvent::ChildInserted);

    QToolTip::add(this, flagstr);
    controller->addListEdit(this);
}

void FlagListEdit::setText( const QString text )
{
    edit->setText(text);
}

bool FlagListEdit::isEmpty( )
{
    return edit->text().isEmpty();
}

QString FlagListEdit::text( )
{
    return edit->text();
}

void FlagListEdit::showListDetails( )
{
    KDialogBase *dia = new KDialogBase(0, "flag_list_edit_dia", true, m_description,
        KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true);

    QBoxLayout *diaLayout = new QVBoxLayout(dia, KDialog::marginHint(), KDialog::spacingHint());
    diaLayout->setAutoAdd(true);

    KEditListBox *elb = new KEditListBox( "", dia );
    dia->setMainWidget(elb);

    elb->insertStringList(QStringList::split(delimiter, text()));

    if (dia->exec() == QDialog::Accepted)
    {
        setText(elb->items().join(delimiter));
    }

    delete dia;
}

void FlagListEdit::appendText( const QString text )
{
    edit->setText(edit->text() + (edit->text().isEmpty()?QString(""):delimiter) + text);
}

QStringList FlagListEdit::flags( )
{
    QStringList fl = QStringList::split(delimiter, text());
    for (QStringList::iterator it = fl.begin(); it != fl.end(); ++it)
    {
        (*it).prepend(flag);
    }
    return fl;
}

FlagSpinEdit::FlagSpinEdit( QWidget * parent, int minVal, int maxVal, int incr, int defaultVal, FlagEditController * controller, const QString & flagstr, const QString & description )
    :QWidget(parent), m_defaultVal(defaultVal), flag(flagstr)
{
    QBoxLayout *topLayout = new QVBoxLayout(this, 0, 1);
    topLayout->addWidget(new QLabel(description, this));

    spb = new QSpinBox(minVal, maxVal, incr, this);
    spb->setValue(defaultVal);
    topLayout->addWidget(spb);

    QApplication::sendPostedEvents(this, QEvent::ChildInserted);

    QToolTip::add(this, flagstr);
    controller->addSpinBox(this);
}

void FlagSpinEdit::setText( const QString text )
{
    spb->setValue(text.toInt());
}

QString FlagSpinEdit::text( )
{
    return QString("%1").arg(spb->value());
}

QString FlagSpinEdit::flags( )
{
    return flag + text();
}

bool FlagSpinEdit::isDefault( )
{
    if (spb->value() == m_defaultVal)
        return true;
    return false;
}


#include "flagboxes.moc"
