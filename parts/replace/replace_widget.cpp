/***************************************************************************
 *   Copyright (C) 2003 by Jens Dagerbo                                    *
 *   jens@krypton.supernet                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//BEGIN Includes

#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <kdebug.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/editor.h>
#include <kmessagebox.h>
#include <klistview.h>

#include <qlayout.h>
#include <qpushbutton.h>
//#include <qprogressbar.h>
//#include <qlistview.h>
#include <qheader.h>
#include <qstringlist.h>
#include <qlineedit.h>
#include <qptrlist.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdir.h>

#include <kdevcore.h>
#include "kdevmainwindow.h"
#include "kdevproject.h"
#include "kdevpartcontroller.h"

#include "replace_part.h"
#include "replace_widget.h"
#include "replacedlg.h"
#include "replaceitem.h"

//END Includes


ReplaceWidget::ReplaceWidget(ReplacePart *part)
        : QWidget(0, "replace widget"), m_part( part ), m_dialog( new ReplaceDlg( this, "replace widget" ) )
{
    QVBoxLayout * layout = new QVBoxLayout( this );
    QHBoxLayout * buttonlayout = new QHBoxLayout( layout );

    _cancel = new QPushButton( "Cancel", this );
    _replace = new QPushButton( "Replace", this );
    //    _progress = new QProgressBar( this );

    _cancel->setEnabled( false );
    _replace->setEnabled( false );

    buttonlayout->addWidget( _cancel );
    buttonlayout->addWidget( _replace );
    //    buttonlayout->addWidget( _progress );

    _listview = new KListView( this );
    _listview->setSorting( -1 );
    _listview->addColumn( "" );
    _listview->header()->hide();
    _listview->setFullWidth();

    layout->addWidget( _listview );

    connect( m_dialog->find_button, SIGNAL( clicked() ), SLOT( find() ) );
    connect( _replace, SIGNAL( clicked() ), SLOT( replace() ) );
    connect( _cancel, SIGNAL( clicked() ), SLOT( clear() ) );
    connect( _listview, SIGNAL( executed(QListViewItem*) ), SLOT( clicked(QListViewItem*) ) );
    //  connect( _listview, SIGNAL( clicked(QListViewItem*) ), SLOT( clicked(QListViewItem*) ) );
}

//BEGIN Slots

void ReplaceWidget::showDialog()
{
    kdDebug(0) << " ******* ReplaceWidget::showDialog()" << endl;

    m_dialog->show();
}

void ReplaceWidget::find()
{
    kdDebug(0) << " ******* ReplaceWidget::find()" << endl;

    if ( m_dialog->modified_files_check->isChecked())
    {
        kdDebug(0) << " *** saving all files *** " << endl;
        m_part->partController()->saveAllFiles();
    }

    kdDebug(0) << "find string: " << m_dialog->find_line->text() << endl;
    kdDebug(0) << "replacement string: " << m_dialog->replacement_line->text() << endl;

    if ( !m_part->project() )	// TODO: change to work on open files when no project is open
        return;

    _listview->clear();
    m_part->mainWindow()->raiseView(this);

    showReplacements( workFiles(), m_dialog->find_line->text(), m_dialog->replacement_line->text() );

    _cancel->setEnabled( true );
    _replace->setEnabled( true );
}

void ReplaceWidget::replace()
{
    kdDebug(0) << " ******* ReplaceWidget::replace()" << endl;

    makeReplacements( m_dialog->find_line->text(), m_dialog->replacement_line->text() );

    clear();
    reloadOpenFiles();
}

void ReplaceWidget::clear()
{
    kdDebug(0) << " ******* ReplaceWidget::clear()" << endl;

    _listview->clear();

    _cancel->setEnabled( false );
    _replace->setEnabled( false );
}

void ReplaceWidget::clicked( QListViewItem * item )
{
    kdDebug(0) << " ******* ReplaceWidget::clicked()" << endl;

    if ( ReplaceItem * rii = dynamic_cast<ReplaceItem*>( item ) )
    {
        kdDebug(0) << "File: " << rii->file() << " Line: " << rii->line() << endl;
        if ( !rii->blockClick() )
        {
            m_part->partController()->editDocument( rii->file(), rii->line() );
        }
    }
}

//END Slots


//BEGIN Helpers

void ReplaceWidget::makeReplacements(QString const & pattern, QString const & replacement )
{
    ReplaceItem * fileitem = static_cast<ReplaceItem*>( _listview->firstChild() );
    
    while ( fileitem )
    {
        if ( fileitem->isOn() )
        {
            kdDebug(0) << " ## " << fileitem->file() << endl;

            QString outfilename = fileitem->file() + "_modified";

            QFile infile( fileitem->file() );
            QFile outfile( outfilename );
            if ( ! ( infile.open( IO_ReadOnly ) && outfile.open( IO_WriteOnly ) ) )
            {
                kdDebug(0) << " **** ERROR opening file! **** " << endl;
                return;
            }
            QTextStream instream( &infile);
            QTextStream outstream( &outfile );

            int line = 0;

            ReplaceItem * lineitem = fileitem->firstChild();
            while ( lineitem )
            {
                if ( lineitem->isOn() )
                {
                    kdDebug(0) << " #### " << lineitem->text() << endl;

                    while ( line < lineitem->line() )
                    {
                        outstream << instream.readLine() << "\n";
                        line++;
                    }
                    // here is the hit
                    // Q_ASSERT( line == lineitem->line() );
                    outstream << instream.readLine().replace( pattern, replacement ) << "\n";
                    line++;
                }

                lineitem = lineitem->nextSibling();
            }

            while ( !instream.atEnd() )
            {
                outstream << instream.readLine() << "\n";
            }

            infile.close();
            outfile.close();

            QDir().rename( outfilename, fileitem->file(), true );
        }
        fileitem = fileitem->nextSibling();
    }
}

void ReplaceWidget::showReplacements( QStringList files, QString pattern, QString replacement )
{
    ReplaceItem::s_listview_done = false;

    ReplaceItem * latestfile = 0;

    QStringList::Iterator it = files.begin();
    while ( it != files.end() )
    {
        ReplaceItem * latestitem = 0;

        QFile file( *it );
        if ( file.open ( IO_ReadOnly ) )
        {
            int line = 0;
            bool firstline = true;
            QTextStream stream ( &file );

            while ( !stream.atEnd() )
            {
                QString s = stream.readLine();

                if ( s.contains( pattern ) > 0 )
                {
                    s.replace( pattern, replacement );

                    if ( firstline )
                    {
                        latestfile = new ReplaceItem( _listview, latestfile, *it );
                        firstline = false;
                    }
                    latestitem = new ReplaceItem( latestfile, latestitem, *it, s.stripWhiteSpace(), line );
                    latestfile->insertItem( latestitem );
                }
                line++;
            }
        }
        ++it;
    }
    ReplaceItem::s_listview_done = true;
}

void ReplaceWidget::reloadOpenFiles()
{
    QPtrList<KParts::Part> * partlist = m_part->partController()->parts();
    KParts::Part * part = partlist->first();
    while ( part != 0)
    {
        if ( KTextEditor::Editor * ed = dynamic_cast<KTextEditor::Editor *>( part ) )
        {
            if ( !ed->isModified() )
            {
                ed->openURL( ed->url().path() );
            }
        }
        part = partlist->next();
    }
}

QStringList ReplaceWidget::workFiles()
{
    if ( m_dialog->all_radio->isChecked() )
    {
        QStringList list = m_part->project()->allFiles();

        QStringList::iterator it = list.begin();
        while ( it != list.end() )
        {
            *it = fullProjectPath( *it );
            ++it;
        }
        return list;
    }
    // else assume m_dialog->all_radio->isChecked()
    return openEditorPaths();
}

QString ReplaceWidget::relativeProjectPath( QString path )
{
    QString project = m_part->project()->projectDirectory() + "/";
    if ( path.left( project.length() ) == project )
    {
        path = path.mid( project.length() );
    }
    return path;
}

QString ReplaceWidget::fullProjectPath( QString path )
{
    QString project = m_part->project()->projectDirectory() + "/";
    if ( path.left( project.length() ) != project )
    {
        path = project + path;
    }
    return path;
}

QStringList ReplaceWidget::openEditorPaths()
{
    return getEditorPaths( false );
}

QStringList ReplaceWidget::modifiedEditorPaths()
{
    return getEditorPaths( true );
}

QStringList ReplaceWidget::getEditorPaths( bool is_modified )
{
    QStringList urls;

    QPtrList<KParts::Part> * partlist = m_part->partController()->parts();
    KParts::Part * part = partlist->first();
    while ( part != 0)
    {
        if ( KTextEditor::Editor * ed = dynamic_cast<KTextEditor::Editor *>( part ) )
        {
            if ( ed->isModified() == is_modified )
            {
                kdDebug(0) << " is_modified = " << is_modified << " - " << ed->url().path() << endl;
                urls.append( ed->url().path() );
            }
        }
        part = partlist->next();
    }
    return urls;
}

//END Helpers

#include "replace_widget.moc"
