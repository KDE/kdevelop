/***************************************************************************
 *   Copyright (C) 2003 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//BEGIN Includes

#include <kparts/part.h>
#include <kparts/componentfactory.h>
#include <klibloader.h>
#include <ktrader.h>
#include <kurl.h>
#include <kurlrequester.h>
#include <kurlcompletion.h>
#include <klineedit.h>
#include <kdebug.h>
#include <kregexpeditorinterface.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/editor.h>
#include <kdevcore.h>
#include <kdevmainwindow.h>
#include <kdevproject.h>
#include <kdevpartcontroller.h>

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qstringlist.h>
#include <qptrlist.h>
#include <qcombobox.h>
#include <qregexp.h>
#include <qdialog.h>
#include <qfile.h>
#include <qtextstream.h>


#include "replace_part.h"
#include "replace_widget.h"
#include "replacedlg.h"
#include "replaceitem.h"
#include "replaceview.h"

//END Includes

namespace
{
//TODO This is the same function as in ../grepview/grepviewwidget.cpp and
//should probably be placed in a common place. For now it seemed like too
//little code to bother with.
QString escape(const QString &str)
{
    QString escaped("[]{}()\\^$?.+-*");
    QString res;

    for (uint i=0; i < str.length(); ++i)
    {
        if (escaped.find(str[i]) != -1)
            res += "\\";
        res += str[i];
    }

    return res;
}
}

ReplaceWidget::ReplaceWidget(ReplacePart *part)
        : QWidget(0, "replace widget"), m_part( part ),
        m_dialog( new ReplaceDlg( this, "replace widget" ) ),
        _regexp_dialog( 0 )
{
    QVBoxLayout * layout = new QVBoxLayout( this );
    QHBoxLayout * buttonlayout = new QHBoxLayout( layout );

    _cancel = new QPushButton( "Cancel", this );
    _replace = new QPushButton( "Replace", this );

    _cancel->setEnabled( false );
    _replace->setEnabled( false );

    buttonlayout->addWidget( _cancel );
    buttonlayout->addWidget( _replace );

    _listview = new ReplaceView( this );
    layout->addWidget( _listview );

    connect( m_dialog->find_button, SIGNAL( clicked() ), SLOT( find() ) );
    connect( _replace, SIGNAL( clicked() ), SLOT( replace() ) );
    connect( _cancel, SIGNAL( clicked() ), SLOT( clear() ) );
//    connect( _listview, SIGNAL( executed(QListViewItem*) ), SLOT( clicked(QListViewItem*) ) );
    connect( _listview, SIGNAL( clicked(QListViewItem*) ), SLOT( clicked(QListViewItem*) ) );
    connect( m_dialog->regexp_button, SIGNAL( clicked() ), SLOT( showRegExpEditor() ) );

    // disable the editor button if the regexp editor isn't installed
    if ( KTrader::self()->query("KRegExpEditor/KRegExpEditor").isEmpty() )
    {
        m_dialog->strings_regexp_radio->disconnect( m_dialog->regexp_button );
    }

    m_dialog->path_urlreq->completionObject()->setMode(KURLCompletion::DirCompletion);
    m_dialog->path_urlreq->setMode( KFile::Directory | KFile::LocalOnly );
}

//BEGIN Slots

void ReplaceWidget::showDialog()
{
    //kdDebug(0) << " ******* ReplaceWidget::showDialog()" << endl;

    if ( ! m_part->project() )
        return; //TODO some feedback here?

    m_dialog->path_urlreq->lineEdit()->setText( fullProjectPath( m_part->project()->projectDirectory() + "/" ) );
    m_dialog->show();
}

void ReplaceWidget::find()
{
    //kdDebug(0) << " ******* ReplaceWidget::find()" << endl;

    QString pattern = escape( m_dialog->find_combo->currentText() );
    QString replacement = m_dialog->replacement_combo->currentText();

    _listview->clear();
    m_part->mainWindow()->raiseView(this);

    QRegExp re;
    re.setCaseSensitive( m_dialog->case_box->isChecked() );
    re.setMinimal( true );

    if ( m_dialog->strings_wholewords_radio->isChecked() )
    {
        pattern = "\\b" + pattern + "\\b";
    }
    else if ( m_dialog->strings_regexp_radio->isChecked() )
    {
        pattern = m_dialog->regexp_combo->currentText();
    }

    if ( ! re.isValid() )
        return; //TODO handle this better, user needs feedback

    re.setPattern( pattern );

    _listview->setReplacementData( re, replacement );

    showReplacements();

    _cancel->setEnabled( true );
    _replace->setEnabled( true );
    //    _replace->setEnabled( false );
}

void ReplaceWidget::replace()
{
    //kdDebug(0) << " ******* ReplaceWidget::replace()" << endl;

    makeReplacements();

    clear();
}

void ReplaceWidget::clear()
{
    //kdDebug(0) << " ******* ReplaceWidget::clear()" << endl;

    _listview->clear();

    _cancel->setEnabled( false );
    _replace->setEnabled( false );
}

void ReplaceWidget::clicked( QListViewItem * item )
{
    //kdDebug(0) << " ******* ReplaceWidget::clicked()" << endl;

    if ( ReplaceItem * rii = dynamic_cast<ReplaceItem*>( item ) )
    {
        //kdDebug(0) << "File: " << rii->file() << " Line: " << rii->line() << endl;
        if ( !rii->blockClick() )
        {
            m_part->partController()->editDocument( rii->file(), rii->line() );
        }
    }
}

void ReplaceWidget::showRegExpEditor()
{
    _regexp_dialog = KParts::ComponentFactory::createInstanceFromQuery<QDialog>( "KRegExpEditor/KRegExpEditor" );

    if ( _regexp_dialog )
    {
        KRegExpEditorInterface *editor =
            static_cast<KRegExpEditorInterface *>( _regexp_dialog->qt_cast( "KRegExpEditorInterface" ) );

        editor->setRegExp( m_dialog->regexp_combo->currentText() );

        if ( _regexp_dialog->exec() == QDialog::Accepted )
        {
            m_dialog->regexp_combo->setCurrentText( editor->regExp() );
        }
    }
}

//END Slots

void ReplaceWidget::showReplacements()
{
    ReplaceItem::s_listview_done = false;

    QStringList files = workFiles();
    QStringList openfiles = openProjectFiles();

    QStringList::ConstIterator it = files.begin();
    while ( it != files.end() )
    {
        if ( openfiles.contains( *it ) )
        {
            if ( KTextEditor::EditInterface * ei = getEditInterfaceForFile( *it ) )
            {
                QString buffer = ei->text();
                QTextIStream stream( &buffer );
                _listview->showReplacementsForFile( stream, *it );
            }
        }
        else
        {
            QFile file( *it );
            if ( file.open ( IO_ReadOnly ) )
            {
                QTextStream stream( &file );
                _listview->showReplacementsForFile( stream, *it );
            }
        }
        ++it;
    }

    ReplaceItem::s_listview_done = true;
}

void ReplaceWidget::makeReplacements()
{
    QStringList openfiles = openProjectFiles();

    ReplaceItem const * fileitem = _listview->firstChild();
    while ( fileitem )
    {
        if ( fileitem->isOn() )
        {
            QString currentfile = fileitem->file();

            //kdDebug(0) << " ## " << currentfile << endl;

            if ( openfiles.contains( currentfile ) )
            {
                if ( KTextEditor::EditInterface * ei = getEditInterfaceForFile( currentfile ) )
                {
                    QString ibuffer = ei->text();
                    QString obuffer;
                    QTextStream istream( &ibuffer, IO_ReadOnly );
                    QTextStream ostream( &obuffer, IO_WriteOnly );

                    _listview->makeReplacementsForFile( istream, ostream, fileitem );

                    ei->removeText( 0, 0, ei->numLines()-1, UINT_MAX );
                    ei->insertText( 0, 0, obuffer );
                }
            }
            else
            {
                QString newfile = currentfile + "_kdevreplace_tempfile";
                QFile ifile( currentfile );
                QFile ofile( newfile );
                if ( ifile.open( IO_ReadOnly ) && ofile.open( IO_WriteOnly ) )
                {
                    QTextStream istream( &ifile );
                    QTextStream ostream( &ofile );

                    _listview->makeReplacementsForFile( istream, ostream, fileitem );

                    ifile.close();
                    ofile.close();

                    QDir().rename( newfile, currentfile, true );
                }
            }
        }

        fileitem = fileitem->nextSibling();
    }
}

//BEGIN Helpers

QStringList const & ReplaceWidget::workFiles()
{
    if ( m_dialog->files_all_radio->isChecked() )
    {
        return allProjectFiles();
    }
    else if ( m_dialog->files_open_radio->isChecked() )
    {
        return openProjectFiles();
    }
    return subProjectFiles( m_dialog->path_urlreq->lineEdit()->text() );
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


QStringList const & ReplaceWidget::allProjectFiles()
{
    _list = m_part->project()->allFiles();

    QStringList::iterator it = _list.begin();
    while ( it != _list.end() )
    {
        *it = fullProjectPath( *it );
        ++it;
    }
    return _list;
}

QStringList const & ReplaceWidget::subProjectFiles( QString const & subpath )
{
    //kdDebug(0) << " ***** ReplaceWidget::subProjectFiles() - subpath == " << subpath << endl;

    QStringList & projectfiles = allProjectFiles();

    QStringList::iterator it = projectfiles.begin();
    while ( it != projectfiles.end() )
    {
        //kdDebug(0) << " ## " << *it << endl;

        if ( (*it).left( subpath.length() ) != subpath)
        {
            //kdDebug(0) << " - removing " << endl;
            it = projectfiles.remove( it );
        }
        else
        {
            //kdDebug(0) << " - keeping " << endl;
            ++it;
        }
    }
    return projectfiles;
}

QStringList const & ReplaceWidget::openProjectFiles()
{
    QStringList projectfiles = allProjectFiles();

    _list.clear();

    QPtrList<KParts::Part> * partlist = m_part->partController()->parts();
    KParts::Part * part = partlist->first();
    while ( part != 0)
    {
        if ( KTextEditor::Editor * ed = dynamic_cast<KTextEditor::Editor *>( part ) )
        {
            QString editorpath = ed->url().path();
            if ( projectfiles.contains( editorpath ) )
            {
                _list.append( editorpath );
            }
        }
        part = partlist->next();
    }
    return _list;
}

KTextEditor::EditInterface * ReplaceWidget::getEditInterfaceForFile( QString const & file )
{
    QPtrList<KParts::Part> * partlist = m_part->partController()->parts();
    KParts::Part * part = partlist->first();
    while ( part != 0)
    {
        if ( KTextEditor::Editor * ed = dynamic_cast<KTextEditor::Editor *>( part ) )
        {
            if ( file == ed->url().path() )
            {
                return dynamic_cast<KTextEditor::EditInterface *>( part );
            }
        }
        part = partlist->next();
    }
    return 0;
}

//END Helpers

#include "replace_widget.moc"
