/***************************************************************************
                          komparemodellist.h
                          -------------------
    begin                : Tue Jun 26 2001
    Copyright 2001-2003 John Firebaugh <jfirebaugh@kde.org>
    Copyright 2001-2005,2009 Otto Bruggeman <bruggie@gmail.com>
    Copyright 2007-2008 Kevin Kofler   <kevin.kofler@chello.at>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KOMPAREMODELLIST_H
#define KOMPAREMODELLIST_H

#include <QtCore/QObject>

#include "diffmodel.h"
#include "diffmodellist.h"
#include "kompare.h"
#include "diff2export.h"


class KAction;
class KTemporaryFile;

class DiffSettings;
class KompareProcess;

namespace Diff2
{

class DIFF2_EXPORT KompareModelList : public QObject
{
	Q_OBJECT
public:
	KompareModelList( DiffSettings* diffSettings, QWidget* widgetForKIO, QObject* parent, const char* name = 0 );
	~KompareModelList();

public:
	void refresh();
	// Swap source with destination and show differences
	void swap();

	/* Comparing methods */
	bool compare();

	bool compare(Kompare::Mode);

	bool openDiff( const QString& diff );

	bool openFileAndDiff();
	bool openDirAndDiff();

	bool saveDiff( const QString& url, QString directory, DiffSettings* diffSettings );
	bool saveAll();

	bool saveDestination( DiffModel* model );

	void setEncoding( const QString& encoding );

	QString recreateDiff() const;

	// This parses the difflines and creates new models
	int parseDiffOutput( const QString& diff );

	// Call this to emit the signals to the rest of the "world" to show the diff
	void show();

	// This will blend the original URL (dir or file) into the diffmodel,
	// this is like patching but with a twist
	bool blendOriginalIntoModelList( const QString& localURL );

	// This mode() method is superfluous now so FIXME
	enum Kompare::Mode    mode()   const { return m_info->mode; };
	const DiffModelList*  models() const { return m_models; };

	int modelCount() const;
	int differenceCount() const;
	int appliedCount() const;

	const DiffModel* modelAt( int i ) const { return m_models->at( i ); };
    DiffModel* modelAt( int i ) { return m_models->at( i ); };
	int              findModel( DiffModel* model ) const { return m_models->indexOf( model ); };

	bool hasUnsavedChanges() const;

	int currentModel() const      { return m_models->indexOf( m_selectedModel ); };
	int currentDifference() const { return m_selectedModel ? m_selectedModel->findDifference( m_selectedDifference ) : -1; };

	const DiffModel* selectedModel() const       { return m_selectedModel; };
	const Difference* selectedDifference() const { return m_selectedDifference; };

	void clear();

private:
	Diff2::DiffModel* firstModel();
	Diff2::DiffModel* lastModel();
	Diff2::DiffModel* prevModel();
	Diff2::DiffModel* nextModel();

	bool setSelectedModel( Diff2::DiffModel* model );

	void updateModelListActions();

protected:
	bool blendFile( DiffModel* model, const QString& lines );

signals:
	void status( Kompare::Status status );
	void setStatusBarModelInfo( int modelIndex, int differenceIndex, int modelCount, int differenceCount, int appliedCount );
	void error( QString error );
	void modelsChanged( const Diff2::DiffModelList* models );
	void setSelection( const Diff2::DiffModel* model, const Diff2::Difference* diff );
	void setSelection( const Diff2::Difference* diff );
	void applyDifference( bool apply );
	void applyAllDifferences( bool apply );
	void applyDifference( const Diff2::Difference* diff, bool apply );
	void diffString( const QString& );
	void updateActions();

public slots:
	void slotSelectionChanged( const Diff2::DiffModel* model, const Diff2::Difference* diff );
	void slotSelectionChanged( const Diff2::Difference* diff );

	void slotApplyDifference( bool apply );
	void slotApplyAllDifferences( bool apply );
	void slotPreviousModel();
	void slotNextModel();
	void slotPreviousDifference();
	void slotNextDifference();

	void slotKompareInfo( struct Kompare::Info* );

protected slots:
	void slotDiffProcessFinished( bool success );
	void slotWriteDiffOutput( bool success );

	void slotActionApplyDifference();
	void slotActionUnApplyDifference();
	void slotActionApplyAllDifferences();
	void slotActionUnapplyAllDifferences();

	/** Save the currently selected destination in a multi-file diff,
	    or the single destination if a single file diff. */
	void slotSaveDestination();

private slots:
	void slotDirectoryChanged( const QString& );
	void slotFileChanged( const QString& );

private: // Helper methods
	bool isDirectory( const QString& url ) const;
	bool isDiff( const QString& mimetype ) const;
	QString readFile( const QString& fileName );

	bool hasPrevModel() const;
	bool hasNextModel() const;
	bool hasPrevDiff() const;
	bool hasNextDiff() const;

	QStringList split( const QString& diff );
	void setDepthAndApplied();

private:
	KTemporaryFile*       m_diffTemp;
	QString               m_diffURL;

	KompareProcess*       m_diffProcess;

	DiffSettings*         m_diffSettings;

	DiffModelList*        m_models;

	DiffModel*            m_selectedModel;
	Difference*           m_selectedDifference;

	int                   m_modelIndex;

	struct Kompare::Info* m_info;

	KAction*              m_applyDifference;
	KAction*              m_unApplyDifference;
	KAction*              m_applyAll;
	KAction*              m_unapplyAll;
	KAction*              m_previousFile;
	KAction*              m_nextFile;
	KAction*              m_previousDifference;
	KAction*              m_nextDifference;

	KAction*              m_save;

	QString               m_encoding;
	QTextCodec*           m_textCodec;

	QWidget*              m_widgetForKIO;
};

} // End of namespace Diff2

#endif
