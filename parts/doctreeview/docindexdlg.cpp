/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qcheckbox.h>
#include <qfile.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qvbuttongroup.h>
#include <kbuttonbox.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kinstance.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstddirs.h>

#include "kdevcore.h"

#include "doctreeviewfactory.h"
#include "doctreeviewpart.h"
#include "docindexdlg.h"


DocIndexDialog::DocIndexDialog(DocTreeViewPart *part, QWidget *parent, const char *name)
    : QDialog(parent, name)
{
    setCaption(i18n("Documentation Index"));

    QLabel *term_label = new QLabel(i18n("Search term:"), this);
    
    term_combo = new KComboBox(true, this);
    term_combo->setFocus();
    QFontMetrics fm(fontMetrics());
    term_combo->setMinimumWidth(fm.width('X')*40);

    KStandardDirs *dirs = DocTreeViewFactory::instance()->dirs();
    QStringList books = dirs->findAllResources("docindices", QString::null, false, true);

    QStringList::Iterator bit;
    for (bit = books.begin(); bit != books.end(); ++bit)
        readIndexFromFile(*bit);
    
    QVButtonGroup *book_group = new QVButtonGroup(this);
    book_group->setExclusive(false);

    QListIterator<DocIndex> iit(indices);
    for (; iit.current(); ++iit) {
        QCheckBox *box = new QCheckBox(iit.current()->title, book_group);
        books_boxes.append(box);

        box->setChecked(true);
        connect( box, SIGNAL(toggled(bool)), this, SLOT(choiceChanged()) );
    }

    QVButtonGroup *category_group = new QVButtonGroup(this);
    category_group->setExclusive(false);

    concept_box = new QCheckBox(i18n("&Concept index"), category_group);
    concept_box->setChecked(true);
    ident_box = new QCheckBox(i18n("&Identifier index"), category_group);
    file_box = new QCheckBox(i18n("&File index"), category_group);
    
    connect( concept_box, SIGNAL(toggled(bool)), this, SLOT(choiceChanged()) );
    connect( ident_box, SIGNAL(toggled(bool)), this, SLOT(choiceChanged()) );
    connect( file_box, SIGNAL(toggled(bool)), this, SLOT(choiceChanged()) );

#if 0
    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(frame, 0);
#endif

    KButtonBox *buttonbox = new KButtonBox(this);
    buttonbox->addStretch();
    QPushButton *ok_button = buttonbox->addButton(i18n("&OK"));
    QPushButton *cancel_button = buttonbox->addButton(i18n("Cancel"));
    ok_button->setDefault(true);
    connect( ok_button, SIGNAL(clicked()), this, SLOT(accept()) );
    connect( cancel_button, SIGNAL(clicked()), this, SLOT(reject()) );
    buttonbox->layout();

    QVBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    layout->addWidget(term_label, 0);
    layout->addWidget(term_combo, 0);
    layout->addWidget(book_group);
    layout->addWidget(category_group);
    layout->addWidget(buttonbox, 0);

    indices.setAutoDelete(true);
    m_part = part;
    choiceChanged();
}


DocIndexDialog::~DocIndexDialog()
{}

    
void DocIndexDialog::readIndexFromFile(const QString &fileName)
{
    QFile f(fileName);
    if (!f.open(IO_ReadOnly)) {
        kdDebug(9002) << "Could not read doc index: " << fileName << endl;
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&f) || doc.doctype().name() != "gideonindex") {
        kdDebug() << "Not a valid gideonindex file: " << fileName << endl;
        return;
    }
    
    f.close();

    kdDebug(9002) << "Parsing: " << fileName << endl;
    
    DocIndex *index = new DocIndex;
    indices.append(index);

    QDomElement docEl = doc.documentElement();
    QDomElement titleEl = docEl.namedItem("title").toElement();
    QDomElement baseEl = docEl.namedItem("base").toElement();
    QDomElement conceptEl = docEl.namedItem("conceptindex").toElement();
    QDomElement identEl = docEl.namedItem("identindex").toElement();
    QDomElement fileEl = docEl.namedItem("fileindex").toElement();
    index->title = titleEl.firstChild().toText().data();
    index->base = baseEl.attribute("href");
    if (!index->base.isEmpty())
        index->base += "/";
    readEntryList(conceptEl, &index->conceptNames, &index->conceptUrls);
    readEntryList(identEl, &index->identNames, &index->identUrls);
    readEntryList(fileEl, &index->fileNames, &index->fileUrls);
}


void DocIndexDialog::readEntryList(const QDomElement &el,
                                   QStringList *nameList, QStringList *urlList)
{
    QDomElement childEl = el.firstChild().toElement();
    while (!childEl.isNull()) {
        if (childEl.tagName() == "entry") {
            nameList->append(childEl.attribute("name"));
            urlList->append(childEl.attribute("url"));
        }

        childEl = childEl.nextSibling().toElement();
    }
}


void DocIndexDialog::accept()
{
    QString term = term_combo->currentText();
    QString url;
    int pos;

    if (term.isEmpty())
        return;
    
    QListIterator<QCheckBox> cit(books_boxes);
    QListIterator<DocIndex> iit(indices);
    for (; cit.current() && iit.current(); ++cit,++iit)
        if ((*cit)->isChecked()) {
            if (concept_box->isChecked())
                if ( (pos = (*iit)->conceptNames.findIndex(term)) != -1) {
                    kdDebug(9002) << "found in concept index of " << (*iit)->title << endl;
                    url = (*iit)->base + (*iit)->conceptUrls[pos];
                    break;
                }
            if (ident_box->isChecked())
                if ( (pos = (*iit)->identNames.findIndex(term)) != -1) {
                    kdDebug(9002) << "found in ident index of " << (*iit)->title << endl;
                    url = (*iit)->base + (*iit)->identUrls[pos];
                    break;
                }
            if (file_box->isChecked())
                if ( (pos = (*iit)->fileNames.findIndex(term)) != -1) {
                    kdDebug(9002) << "found in file index of " << (*iit)->title << endl;
                    url = (*iit)->base + (*iit)->fileUrls[pos];
                    break;
                }
        }

    if (url.isEmpty()) {
        KMessageBox::sorry(this, i18n("Term not found in the indices."));
        return;
    }
    
    m_part->core()->gotoDocumentationFile(KURL(url));
    
    QDialog::accept();
}


void DocIndexDialog::choiceChanged()
{
    QStringList completions;
    
    QListIterator<QCheckBox> cit(books_boxes);
    QListIterator<DocIndex> iit(indices);
    for (; cit.current() && iit.current(); ++cit,++iit)
        if ((*cit)->isChecked()) {
            if (concept_box->isChecked())
                completions += (*iit)->conceptNames;
            if (ident_box->isChecked())
                completions += (*iit)->identNames;
            if (file_box->isChecked())
                completions += (*iit)->fileNames;
        }
    
    term_combo->completionObject()->setItems(completions);
}
#include "docindexdlg.moc"
