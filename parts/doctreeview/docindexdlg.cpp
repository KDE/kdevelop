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

#include "docindexdlg.h"

#include <qapplication.h>
#include <qcheckbox.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qvbuttongroup.h>
#include <qregexp.h>
#include <kbuttonbox.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kinstance.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kprocess.h>
#include <kdeversion.h>
#include <kstdguiitem.h>

#include "kdevcore.h"
#include "kdevpartcontroller.h"
#include "domutil.h"

#include "misc.h"
#include "doctreeviewfactory.h"
#include "doctreeviewpart.h"


DocIndexDialog::DocIndexDialog(DocTreeViewPart *part, QWidget *parent, const char *name)
    : QDialog(parent, name)
{
    setCaption(i18n("Documentation Index"));

    QLabel *term_label = new QLabel(i18n("Search term:"), this);

    term_combo = new KComboBox(true, this);
    term_combo->setFocus();
    QFontMetrics fm(fontMetrics());
    term_combo->setMinimumWidth(fm.width('X')*40);

    QApplication::setOverrideCursor(waitCursor);

    readKDocIndex();

    KStandardDirs *dirs = DocTreeViewFactory::instance()->dirs();
    QStringList books = dirs->findAllResources("docindices", QString::null, false, true);

    QStringList::Iterator bit;
    for (bit = books.begin(); bit != books.end(); ++bit)
        readIndexFromFile(*bit);

    QApplication::restoreOverrideCursor();

    QVButtonGroup *book_group = new QVButtonGroup(this);
    book_group->setExclusive(false);

    QPtrListIterator<DocIndex> iit(indices);
    for (; iit.current(); ++iit) {
        QCheckBox *box = new QCheckBox(iit.current()->title, book_group);
        box->setChecked(true);
        books_boxes.append(box);
        connect( box, SIGNAL(toggled(bool)), this, SLOT(choiceChanged()) );
    }

    QVButtonGroup *category_group = new QVButtonGroup(this);
    category_group->setExclusive(false);

    concept_box = new QCheckBox(i18n("&Concept index"), category_group);
    concept_box->setChecked(true);
    ident_box = new QCheckBox(i18n("&Identifier index"), category_group);
    ident_box->setChecked(true);
    file_box = new QCheckBox(i18n("&File index"), category_group);
    file_box->setChecked(true);

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
#if KDE_IS_VERSION( 3, 2, 90 )
    QPushButton *ok_button = buttonbox->addButton(KStdGuiItem::ok());
    QPushButton *cancel_button = buttonbox->addButton(KStdGuiItem::cancel());
#else
    QPushButton *ok_button = buttonbox->addButton(i18n("&OK"));
    QPushButton *cancel_button = buttonbox->addButton(i18n("Cancel"));
#endif

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

    if (m_part->project())
        readConfig();
}


DocIndexDialog::~DocIndexDialog()
{}


void DocIndexDialog::projectChanged()
{
    if (m_part->project())
        readConfig();
}


void DocIndexDialog::lookup(const QString &str)
{
    term_combo->lineEdit()->setText(str);
}


void DocIndexDialog::readConfig()
{
    QDomDocument &dom = *m_part->projectDom();
    QDomElement docEl = dom.documentElement();
    QDomElement doctreeviewEl = docEl.namedItem("kdevdoctreeview").toElement();

    QStringList indexbooks;
    QDomElement indexbooksEl = doctreeviewEl.namedItem("indexbooks").toElement();
    QDomElement bookEl = indexbooksEl.firstChild().toElement();
    while (!bookEl.isNull()) {
        if (bookEl.tagName() == "book")
            indexbooks << bookEl.firstChild().toText().data();
        bookEl = bookEl.nextSibling().toElement();
    }

    QPtrListIterator<QCheckBox> cit(books_boxes);
    QPtrListIterator<DocIndex> iit(indices);
    for (; cit.current() && iit.current(); ++cit,++iit)
        (*cit)->setChecked(indexbooks.isEmpty() || indexbooks.contains(iit.current()->indexName));

    concept_box->setChecked(DomUtil::readBoolEntry(dom, "/kdevdoctreeview/categories/concept"));
    ident_box->setChecked(DomUtil::readBoolEntry(dom, "/kdevdoctreeview/categories/identifier"));
    file_box->setChecked(DomUtil::readBoolEntry(dom, "/kdevdoctreeview/categories/file"));
}


void DocIndexDialog::storeConfig()
{
    QDomDocument &dom = *m_part->projectDom();
    QDomElement docEl = dom.documentElement();
    QDomElement doctreeviewEl = docEl.namedItem("kdevdoctreeview").toElement();

    QDomElement indexbooksEl = doctreeviewEl.namedItem("indexbooks").toElement();
    if (indexbooksEl.isNull()) {
        indexbooksEl = dom.createElement("indexbooks");
        doctreeviewEl.appendChild(indexbooksEl);
    }

    // Clear old entries
    while (!indexbooksEl.firstChild().isNull())
        indexbooksEl.removeChild(indexbooksEl.firstChild());

    QPtrListIterator<QCheckBox> cit(books_boxes);
    QPtrListIterator<DocIndex> iit(indices);
    for (; cit.current() && iit.current(); ++cit,++iit)
        if ((*cit)->isChecked()) {
            QDomElement bookEl = dom.createElement("book");
            bookEl.appendChild(dom.createTextNode((*iit)->indexName));
            indexbooksEl.appendChild(bookEl);
            kdDebug() << "Appending " << ((*iit)->indexName) << endl;
        }

    DomUtil::writeBoolEntry(dom, "/kdevdoctreeview/categories/concept", concept_box->isChecked());
    DomUtil::writeBoolEntry(dom, "/kdevdoctreeview/categories/identifier", ident_box->isChecked());
    DomUtil::writeBoolEntry(dom, "/kdevdoctreeview/categories/file", file_box->isChecked());
}


void DocIndexDialog::readKDocIndex()
{
    DocIndex *index = new DocIndex;
    indices.append(index);

    index->indexName = "qt";
    index->title = i18n("Qt/KDE API");

    QStringList itemNames, fileNames, hiddenNames;
    DocTreeViewTool::getAllLibraries(&itemNames, &fileNames);
    DocTreeViewTool::getHiddenLibraries(&hiddenNames);

    QStringList::Iterator it;
    for (it = fileNames.begin(); it != fileNames.end(); ++it)
        if (!hiddenNames.contains(*it)) {
            FILE *f;
            if ((*it).right(3) != QString::fromLatin1(".gz")) {
                if ( (f = fopen(QFile::encodeName( *it ).data(), "r")) != 0) {
                    readKDocEntryList(f, &index->identNames, &index->identUrls);
                    fclose(f);
                }
            } else {
                QString cmd = "gzip -c -d ";
#if (KDE_VERSION > 305)
                cmd += KProcess::quote(*it);
#else
                cmd += KShellProcess::quote(*it);
#endif
                cmd += " 2>/dev/null";
                if ( (f = popen(QFile::encodeName(cmd), "r")) != 0) {
                    readKDocEntryList(f, &index->identNames, &index->identUrls);
                    pclose(f);
                }
            }
        }
}


void DocIndexDialog::readKDocEntryList(FILE *f,
                                       QStringList *nameList, QStringList *urlList)
{
    char buf[1024];
    int pos0;
    QString classname, membername, base, filename;

    while (fgets(buf, sizeof buf, f)) {
        QString s = buf;
        if (s.left(pos0=11) == "<BASE URL=\"") {
            int pos2 = s.find("\">", pos0);
            if (pos2 != -1)
                base = s.mid(pos0, pos2-pos0);
        }
        else if (s.left(pos0=9) == "<C NAME=\"") {
            int pos1 = s.find("\" REF=\"", pos0);
            if (pos1 == -1)
                continue;
                int pos2 = s.find("\">", pos1+7);
                if (pos2 == -1)
                    continue;
                classname = s.mid(pos0, pos1-pos0);
                filename = s.mid(pos1+7, pos2-(pos1+7));
                filename.replace(QRegExp("::"), "__");
                (*nameList) << classname;
                (*urlList) << (base + "/" + filename);
        }
      else if (s.left(pos0=9) == "<M NAME=\"" || s.left(pos0=10) == "<ME NAME=\"")
          {
              int pos1 = s.find("\" REF=\"", pos0);
              if (pos1 == -1)
                  continue;
              int pos2 = s.find("\">", pos1+7);
              if (pos2 == -1)
                  continue;

              // Long version: membername = classname + "::" + s.mid(pos0, pos1-pos0);
              membername = s.mid(pos0, pos1-pos0);
              filename = s.mid(pos1+7, pos2-(pos1+7));
              filename.replace(QRegExp("::"), "__");
              (*nameList) << (membername + " (" + classname + ")");
              (*urlList) << (base + "/" + filename);
          }
    }
}


void DocIndexDialog::readIndexFromFile(const QString &fileName)
{
    QFileInfo fi(fileName);
    QString name = fi.baseName();

    QFile f(fileName);
    if (!f.open(IO_ReadOnly)) {
        kdDebug(9002) << "Could not read doc index: " << fileName << endl;
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&f) || doc.doctype().name() != "kdevelopindex") {
        kdDebug() << "Not a valid kdevelopindex file: " << fileName << endl;
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
    index->indexName = name;
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

    QPtrListIterator<QCheckBox> cit(books_boxes);
    QPtrListIterator<DocIndex> iit(indices);
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

    m_part->partController()->showDocument(KURL(url));

    if (m_part->project())
        storeConfig();

    QDialog::accept();
}


void DocIndexDialog::choiceChanged()
{
    QStringList completions;

    QPtrListIterator<QCheckBox> cit(books_boxes);
    QPtrListIterator<DocIndex> iit(indices);
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
