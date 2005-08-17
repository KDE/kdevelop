/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MakeItem_h
#define MakeItem_h

#include <qstring.h>

enum EOutputLevel
{
	// appropriate to the ID's in the button group of settingswidget.ui
	eVeryShort = 0
	,eShort
	,eFull
};

class MakeItem
{
public:
	enum Type { Normal, Error, Warning, Diagnostic };
	MakeItem();
	MakeItem( const QString& text );
	virtual ~MakeItem();

  enum DisplayModes
  {
    DelayedDisplay = 0, // item can be displayed later
    ImmDisplay = 1,     // item has to be displayed ASAP
    Append = 2          // item's text can be appended (append has been overloaded)
  };
  virtual int displayMode() const { return ImmDisplay; }
	virtual bool append( const QString& ) { return false; }
	virtual Type type() { return Diagnostic; }
	virtual bool visible( EOutputLevel level ) { return level > eVeryShort; }
	virtual QString text( EOutputLevel );
	virtual QString formattedText( EOutputLevel, bool bright_bg );
	QString icon();
	QString color( bool bright_bg );

	static QString br();

	QString m_text;
};

class CommandItem : public MakeItem
{
public:
	CommandItem(const QString command)
		: MakeItem( command )
	{}

	Type type() { return Diagnostic; }
	virtual bool visible( EOutputLevel ) { return true; }
};

class ExitStatusItem : public MakeItem
{
public:
	ExitStatusItem( bool normalExit, int exitStatus );

	Type type() { return m_normalExit && m_exitStatus == 0 ? Diagnostic : Error; }
	virtual bool visible( EOutputLevel ) { return true; }
	QString text( EOutputLevel level );

private:
	bool m_normalExit;
	int m_exitStatus;
};

class DirectoryItem : public MakeItem
{
public:
	DirectoryItem( const QString& dir, const QString& text )
		: MakeItem( text )
		, directory( dir )
	{}

	Type type() { return Diagnostic; }

	static void setShowDirectoryMessages( bool show ) { m_showDirectoryMessages = show; }
	static bool getShowDirectoryMessages() { return m_showDirectoryMessages; }

	QString directory;

protected:
	static bool m_showDirectoryMessages;
};

class EnteringDirectoryItem : public DirectoryItem
{
public:
	EnteringDirectoryItem( const QString& dir, const QString& text )
		: DirectoryItem( dir, text )
	{}
	bool visible( EOutputLevel )
	{
		return m_showDirectoryMessages;
	}

	virtual QString text( EOutputLevel );
};

class ExitingDirectoryItem : public DirectoryItem
{
public:
	ExitingDirectoryItem( const QString& dir, const QString& text )
		: DirectoryItem( dir, text )
	{}
	bool visible( EOutputLevel level )
	{
		return m_showDirectoryMessages && level > eVeryShort;
	}

	virtual QString text( EOutputLevel );
};

namespace KTextEditor { class View; class Document; }

class ErrorItem : public MakeItem
{
public:
	ErrorItem( const QString& fn, int ln, const QString& tx, const QString& line, bool isWarning, const QString& compiler );
	virtual ~ErrorItem();

	Type type() { return m_isWarning ? Warning : Error; }
        
	virtual bool append( const QString& text );
	virtual int displayMode() const { return DelayedDisplay | Append; }
	virtual bool visible( EOutputLevel ) { return true; }

	QString fileName;
	int lineNum;
	QString m_error;
	KTextEditor::View* m_cursor;
	KTextEditor::Document* m_doc;
	bool m_isWarning;
	QString m_compiler;
};

class ActionItem : public MakeItem
{
public:
	ActionItem( const QString& action, const QString& file, const QString& tool, const QString& line )
		: MakeItem( line )
		, m_action( action )
		, m_file( file )
		, m_tool( tool )
	{}

	virtual bool visible( EOutputLevel ) { return true; }
	virtual QString text( EOutputLevel level );

	QString m_action;
	QString m_file;
	QString m_tool;
};

#endif
