/***************************************************************************
 *   Copyright (C) 2007 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "snippet.h"
#include "globals.h"

#include <QtDebug>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QDir>

#include "snippetstore.h"
#include "snippetrepository.h"
#include "snippetvariablesubst.h"

Snippet::Snippet(QString filename, SnippetRepository* repo)
    : QStandardItem(filename), repo_(repo), name_(filename)
{
    // append ourself to the given parent repo
    repo->appendRow( this );

    // tell the new snippet to get it's data from the file
    QTimer::singleShot(0, this, SLOT(slotSyncSnippet()));
}

Snippet::~Snippet()
{
}

void Snippet::slotSyncSnippet()
{
    if (!QFile::exists( getFileName() ))
        return;

    QFile f( getFileName() );
    if(f.open(QIODevice::ReadOnly)) {
        QTextStream input( &f );
        setRawData( input.readAll() );
    }
}

void Snippet::save()
{
    QFile f( getFileName() );
    if(f.open(QIODevice::WriteOnly)) {
        QTextStream input( &f );
        input << snippetText_;

        foreach(QString keyword, keywords_) {
            input << SNIPPET_METADATA << " keyword="<<keyword<<endl;
        }
    }
}

const QString Snippet::getFileName()
{
    return repo_->getLocation() + QDir::separator() + name_;
}

void Snippet::changeName(const QString& newName)
{
    // If there is no name yet, e.g. the snippet has just been created
    // we don't need to care about changing the filename
    if (name_.isEmpty()) {
        name_ = newName;
        setText( newName );
        return;
    }

    // In case the user changed the name of the snippet,
    // we need to move the current file.
    if (name_ != newName) {
        QString origFileName = getFileName();
        name_ = newName;
        QString newFileName = getFileName();

        if ( QFile::exists( origFileName ) ) {
            bool ok = QFile::rename(origFileName, newFileName);
            if (ok) {
                setText( newName );
            }
        } else {
            setText( newName );
            save();
        }
    }

    // no need to do anything if the name didn't change
}

void Snippet::setRawData(QString data)
{
    QStringList rows = data.split( QRegExp("[\\r\\n]+") );
    QStringList metadata;

    QString newText;

    // A snippet file can contain meta information
    // which needs to be separeted from the snippet's text
    QStringListIterator it(rows);
    while (it.hasNext()) {
        QString str = it.next();

        if (str.startsWith(SNIPPET_METADATA)) {
            metadata << str;
        } else {
            newText += str;
            if (it.hasNext()) {
                newText += SNIPPET_END_OF_LINE;
            }
        }
    }

    setSnippetText( newText );
    keywords_.clear();

    if (metadata.count() > 0) {
        // if there is meta information, call handleMetaData() which is 
        // capable of parsing them
        handleMetaData( metadata );
    }
}

void Snippet::handleMetaData(QStringList& metadata)
{
    //Each line looks like this:
    //##META## keyword = abcdef
    QRegExp rx("(\\w+)\\s*=\\s*(\\w*)");

    QString str;
    foreach(str, metadata) {
        // strip off the metadata prefix from the lines
        str.remove(QString(SNIPPET_METADATA));

        // now use the regexp to get the name and the value from this metadata-line
        int pos = rx.indexIn( str );
        if (pos > -1) {
            QString name = rx.cap(1);
            QString value = rx.cap(2);

            // keywords get moved into the keywords list
            if (name.toLower() == "keyword") {
                keywords_ << value;
            }
        }
    }
}

int Snippet::initVariableModel(SnippetVariableModel& model)
{
    // The set helps us to ensure that each variable gets found only once
    QSet<QString> set;

    // This regexp finds all variables in the snippet
    QRegExp rx("\\$(\\w+)\\$");
    int pos = 0;
    while ((pos = rx.indexIn(snippetText_, pos)) != -1) {
        set << rx.cap(1);
        pos += rx.matchedLength();
    }

    // Copy the variable names into the given model
    int count = 0;
    foreach (QString str, set) {
        model.appendVariable( str );
        count++;
    }

    return count;
}

const QString Snippet::interpretSnippet()
{
    SnippetVariableModel variables;

    // First we need to check if there are variables in the snippet
    // If so, they get copied into the model
    int count = initVariableModel( variables );

    QString snippet = snippetText_;

    // first we need to replace the double-delimiter into a single delimiter
    snippet.replace( "$$", "$" );

    if (count > 0) {
        // open the SnippetVariables dialog and give it our model
        SnippetVariables dlg( &variables );
        if ( dlg.exec() == QDialog::Accepted ) {
            // If the user accepted the dialog, it holds a list of variables and the 
            // values the user wants to substitute

            SnippetVariableItem* variable;
            foreach(variable, variables.getVariableList()) {
                snippet.replace( "$"+variable->name()+"$", variable->value() );
            }
        } else {
            // the user canceled the dialog; return an empty string
            return QString();
        }
    }

    // In case the snippet does not contain variables, just return the text
    return snippet;
}

void Snippet::removeSnippetFile()
{
    bool ok = QFile::remove( getFileName() );
    if (ok) {
        // if the file has been removed, also remove the Snippet from the model
        QStandardItem::parent()->removeRows( row(), 1 );
    }
}

#include "snippet.moc"
// kate: space-indent on; indent-width 4; tab-width 4; indent-mode cstyle; replace-tabs on; auto-insert-doxygen on;
