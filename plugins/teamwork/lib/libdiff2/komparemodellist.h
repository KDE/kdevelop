/***************************************************************************
                          komparemodellist.h  -  description
                             -------------------
    begin                : Tue Jun 26 2001
    copyright            : (C) 2001-2003 by John Firebaugh
                           and Otto Bruggeman
    email                : jfirebaugh@kde.org
                           otto.bruggeman@home.nl
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

#include <qobject.h>

#include "diffmodel.h"
#include "diffmodellist.h"
#include "kompare.h"


class KAction;
class KDirWatch;
class K3TempFile;

class DiffSettings;
class KompareProcess;

namespace Diff2
{

class KompareModelList : public QObject
{
	Q_OBJECT
public:
	KompareModelList( DiffSettings* diffSettings, struct Kompare::Info& info, QObject* parent = 0, const char* name = 0 );
	~KompareModelList();

public:
	// Swap source with destination and show differences
	void swap();

	/* Comparing methods */
	bool compare( const QString& source, const QString& destination );

	bool compareFiles( const QString& source, const QString& destination );
	bool compareDirs( const QString& source, const QString& destination );

	bool openDiff( const QString& diff );

	bool openFileAndDiff( const QString& file, const QString& diff );
	bool openDirAndDiff( const QString& dir, const QString& diff );

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

	enum Kompare::Mode    mode()   const { return m_info.mode; };
	const DiffModelList*  models() const { return m_models; };

	int modelCount() const;
	int differenceCount() const;
	int appliedCount() const;

	const DiffModel* modelAt( int i ) const { return *( m_models->at( i ) ); };
	int              findModel( DiffModel* model ) const { return findItem( model, m_models ); };

	bool isModified() const;

	int currentModel() const      { return findItem( m_selectedModel, m_models  ); };
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

	// Emits true when m_noOfModified > 0, false when m_noOfModified == 0
	void setModified( bool modified );

public slots:
	void slotSelectionChanged( const Diff2::DiffModel* model, const Diff2::Difference* diff );
	void slotSelectionChanged( const Diff2::Difference* diff );

	void slotApplyDifference( bool apply );
	void slotApplyAllDifferences( bool apply );
	void slotPreviousModel();
	void slotNextModel();
	void slotPreviousDifference();
	void slotNextDifference();

	// This slot is called by the diffmodels whenever their status changes to modified or unmodified
	void slotSetModified( bool modified );

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

private:
	K3TempFile*            m_diffTemp;
	QString               m_diffURL;

	KompareProcess*       m_diffProcess;

	DiffSettings*         m_diffSettings;

	DiffModelList*        m_models;

	QString               m_source;
	QString               m_destination;

	DiffModel*            m_selectedModel;
	Difference*           m_selectedDifference;

	KDirWatch*            m_dirWatch;
	KDirWatch*            m_fileWatch;

	int                   m_noOfModified;
	unsigned int          m_modelIndex;

	struct Kompare::Info& m_info;

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
};

} // End of namespace Diff2

#endif
