/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "snippetrepository.h"

#include "snippet.h"

#include <QTimer>
#include <QFile>
#include <QFileInfo>

#include <KIcon>
#include <KDebug>

#include <QDomDocument>
#include <QDomElement>
#include <KStandardDirs>

#include <KMessageBox>
#include <KLocalizedString>
#include <QApplication>

SnippetRepository::SnippetRepository(const QString& file)
 : QStandardItem(i18n("<empty repository>")), m_file(file)
{
    setIcon( KIcon("folder") );

    if ( QFile::exists(file) ) {
        // Tell the new repository to load it's snippets
        QTimer::singleShot(0, this, SLOT(slotParseFile()));
    }

    kDebug() << "created new snippet repo" << file << this;
}

SnippetRepository::~SnippetRepository()
{
    // remove all our children from both the model and our internal data structures
    removeRows( 0, rowCount() );
}

QString SnippetRepository::getFileForName(const QString& name)
{
    return KGlobal::dirs()->locateLocal( "data", "kate/plugins/katesnippets_tng/data/" + name + ".xml" );
}

const QString& SnippetRepository::getFile() const
{
    return m_file;
}

QString SnippetRepository::authors() const
{
    return m_authors;
}

void SnippetRepository::setAuthors(const QString& authors)
{
    m_authors = authors;
}

QStringList SnippetRepository::fileTypes() const
{
    return m_filetypes;
}

void SnippetRepository::setFileTypes(const QStringList& filetypes)
{
    if ( filetypes.contains("*") ) {
        m_filetypes.clear();
    } else {
        m_filetypes = filetypes;
    }
}

QString SnippetRepository::license() const
{
    return m_license;
}

void SnippetRepository::setLicense(const QString& license)
{
    m_license = license;
}

void SnippetRepository::remove()
{
    QFile::remove(m_file);
    deleteLater();
}

///copied code from snippets_tng/lib/completionmodel.cpp
///@copyright 2009 Joseph Wenninger <jowenn@kde.org>
static void addAndCreateElement(QDomDocument& doc, QDomElement& item, const QString& name, const QString &content)
{
    QDomElement element=doc.createElement(name);
    element.appendChild(doc.createTextNode(content));
    item.appendChild(element);
}

void SnippetRepository::save()
{
    ///based on the code from snippets_tng/lib/completionmodel.cpp
    ///@copyright 2009 Joseph Wenninger <jowenn@kde.org>
    /*
    <snippets name="Testsnippets" filetype="*" authors="Joseph Wenninger" license="BSD">
        <item>
            <displayprefix>prefix</displayprefix>
            <match>test1</match>
            <displaypostfix>postfix</displaypostfix>
            <displayarguments>(param1, param2)</displayarguments>
            <fillin>This is a test</fillin>
        </item>
        <item>
            <match>testtemplate</match>
            <fillin>This is a test ${WHAT} template</fillin>
        </item>
    </snippets>
    */
    QDomDocument doc;

    QDomElement root = doc.createElement("snippets");
    root.setAttribute("name", text());
    root.setAttribute("filetypes", m_filetypes.isEmpty() ? "*" : m_filetypes.join(";"));
    root.setAttribute("authors", m_authors);
    root.setAttribute("license", m_license);

    doc.appendChild(root);

    for ( int i = 0; i < rowCount(); ++i ) {
        Snippet* snippet = dynamic_cast<Snippet*>(child(i));
        if ( !snippet ) {
            continue;
        }
        QDomElement item = doc.createElement("item");
        addAndCreateElement(doc, item, "displayprefix", snippet->prefix());
        addAndCreateElement(doc, item, "match", snippet->text());
        addAndCreateElement(doc, item, "displaypostfix", snippet->postfix());
        addAndCreateElement(doc, item, "displayarguments", snippet->arguments());
        addAndCreateElement(doc, item, "fillin", snippet->snippet());
        root.appendChild(item);
    }
    //KMessageBox::information(0,doc.toString());
    QFileInfo fi(m_file);
    kDebug() << fi.fileName();
    QString outname = KGlobal::dirs()->locateLocal( "data", "kate/plugins/katesnippets_tng/data/" + fi.fileName() );
    if ( m_file != outname) {
        QFileInfo fiout(outname);
//      if (fiout.exists()) {
// there could be cases that new new name clashes with a global file, but I guess it is not that often.
        bool ok = false;
        for (int i=0;i<1000;i++) {
            outname = KGlobal::dirs()->locateLocal( "data", "kate/plugins/katesnippets_tng/data/"+QString("%1_").arg(i)+fi.fileName());
            if (QFile::exists(outname)) {
                ok = true;
                break;
            }
        }
        if (!ok) {
            KMessageBox::error(0,i18n("You have edited a data file not located in your personal data directory, but a suitable filename could not be generated for storing a clone of the file within your personal data directory."));
            return;
        } else {
            KMessageBox::information(0,i18n("You have edited a data file not located in your personal data directory; as such, a renamed clone of the original data file has been created within your personal data directory."));
        }
//       } else
//         KMessageBox::information(0,i18n("You have edited a data file not located in your personal data directory, creating a clone of the data file in your personal data directory"));
    }

    QFile outfile(outname);
    if (!outfile.open(QIODevice::WriteOnly)) {
        KMessageBox::error(0, i18n("Output file '%1' could not be opened for writing", outname));
        return;
    }
    outfile.write(doc.toByteArray());
    outfile.close();
}

void SnippetRepository::slotParseFile()
{
    ///based on the code from snippets_tng/lib/completionmodel.cpp
    ///@copyright 2009 Joseph Wenninger <jowenn@kde.org>

    QFile f(m_file);

    if ( !f.open(QIODevice::ReadOnly) ) {
        KMessageBox::error( QApplication::activeWindow(), i18n("Cannot open snippet repository %1.", m_file) );
        return;
    }

    QDomDocument doc;
    QString errorMsg;
    int line, col;
    bool success = doc.setContent(&f, &errorMsg, &line, &col);
    f.close();

    if (!success) {
        KMessageBox::error( QApplication::activeWindow(),
                           i18n("<qt>The error <b>%4</b><br /> has been detected in the file %1 at %2/%3</qt>",
                                m_file, line, col, i18nc("QXml", errorMsg.toUtf8())) );
        return;
    }

    // parse root item
    const QDomElement& docElement = doc.documentElement();
    if (docElement.tagName() != "snippets") {
        KMessageBox::error( QApplication::activeWindow(), i18n("Invalid XML snippet file: %1", m_file) );
        return;
    }
    setLicense(docElement.attribute("license"));
    setAuthors(docElement.attribute("authors"));
    setFileTypes(docElement.attribute("filetypes").split(';', QString::SkipEmptyParts));
    setText(docElement.attribute("name"));

    // parse children, i.e. <item>'s
    const QDomNodeList& nodes = docElement.childNodes();
    for(int i = 0; i < nodes.size(); ++i ) {
        const QDomNode& node = nodes.at(i);
        if ( !node.isElement() ) {
            continue;
        }
        const QDomElement& item = node.toElement();
        if ( item.tagName() != "item" ) {
            continue;
        }
        Snippet* snippet = new Snippet;
        const QDomNodeList& children = node.childNodes();
        for(int j = 0; j < children.size(); ++j) {
            const QDomNode& childNode = children.at(j);
            if ( !childNode.isElement() ) {
                continue;
            }
            const QDomElement& child = childNode.toElement();
            if ( child.tagName() == "match" ) {
                snippet->setText(child.text());
            } else if ( child.tagName() == "fillin" ) {
                snippet->setSnippet(child.text());
            } else if ( child.tagName() == "displayprefix" ) {
                snippet->setPrefix(child.text());
            } else if ( child.tagName() == "displaypostfix" ) {
                snippet->setPostfix(child.text());
            } else if ( child.tagName() == "displayarguments" ) {
                snippet->setArguments(child.text());
            }
        }
        // require at least a non-empty name and snippet
        if ( snippet->text().isEmpty() || snippet->snippet().isEmpty() ) {
            delete snippet;
            continue;
        } else {
            appendRow(snippet);
        }
    }
}

#include "snippetrepository.moc"
