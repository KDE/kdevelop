/***************************************************************************
                          persistantclassstore.h  -  description
                             -------------------
    begin                : Mon Mar 29 1999
    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@syncom.se
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#ifndef _PERSISTANTCLASSTORE_H_
#define _PERSISTANTCLASSTORE_H_

#define VERSION_FILE_FORMAT "0.1"

#include "classstore.h"

class QFile;

/** This class handles all persistant storage of classes,
 * global functions and variables.
 * @author Jonas Nordin
 */
class PersistantClassStore : public ClassStore
{

public: // Constructor & Destructor

   PersistantClassStore();
  ~PersistantClassStore();

public: // Public attributes

  /** Path where the persistant store file will be put. */
  QString m_strPath;

  /** The filename. */
  QString m_strFileName;

  /** Is the file opened? */
  bool m_bIsOpen;

public: // Public methods

  /** Set the path where the persistant store file should be stored.
   * @param aPath Path to the persistant store file.
   */
  void setPath( const QString &aPath );

  /** Set the name of the file to read/write. 
   * @param aFileName Name of the persistant store file.
   */
  void setFileName( const QString &aFileName );

  /** Open the file. */
  bool open ( const QString &aFileName, int nMode );

  /** Close the file. */
  void close();

  /** Has the store been created? */
  bool exists();

  /** Stores all data in a binary file */
  void storeAll();

  /** Restores all data from a binary file */
  void restoreAll();

protected: // Protected methods

  /** Store a class in the persistant store.
   * @param aClass The class to store in the persistant store.
   */
  void storeClass ( ParsedClass *pClass );

  /** Store a scope in the persistant store.
   * @param aScope The class to store in the persistant store.
   */
  void storeScope ( ParsedScopeContainer* pScope );

  /** Store a Method in the persistant store.
   * @param aMethod The class to store in the persistant store.
   */
  void storeMethod ( ParsedMethod* pMethod );

  /** Store a attribute in the persistant store.
   * @param aAttribute The class to store in the persistant store.
   */
  void storeAttribute ( ParsedAttribute* pAttribute );

  /** Store a struct in the persistant store.
   * @param aStruct The class to store in the persistant store.
   */
  void storeStruct ( ParsedStruct* pStruct );


private: // Private attributes

	QFile* m_pFile;
	QDataStream* m_pStream;

};

#endif 
