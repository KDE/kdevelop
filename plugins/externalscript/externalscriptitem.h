/*
    This plugin is part of KDevelop.

    Copyright (C) 2010 Milian Wolff <mail@milianw.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef EXTERNALSCRIPTITEM_H
#define EXTERNALSCRIPTITEM_H

#include <QStandardItemModel>

class KAction;

/**
 * NOTE: use @c text() and @c setText() to define the label/name of the external script.
 */
class ExternalScriptItem : public QStandardItem
{
public:
  ExternalScriptItem();

  /**
   * @return The command to execute. Working dir will be that of the active document.
   */
  QString command() const;
  /**
   * Sets the command to execute. Working dir will be that of the active document.
   */
  void setCommand( const QString& command );

  enum SaveMode {
    /// Nothing needs to be saved.
    SaveNone,
    /// Currently active document gets saved.
    SaveCurrentDocument,
    /// All opened documents get saved.
    SaveAllDocuments
  };
  /**
   * @return @c SaveMode that decides what document should be saved before executing this script.
   */
  SaveMode saveMode() const;
  /**
   * Sets the @c SaveMode that decides what document should be saved before executing this script.
   */
  void setSaveMode( SaveMode mode );

  enum ReplaceMode {
    /// Nothing gets replaced in the active document.
    ReplaceNone,
    /// Current selection gets replaced in the active document.
    /// If no selection exists, the whole document gets replaced.
    ReplaceSelection,
    /// The whole contents of the active document gets replaced.
    ReplaceDocument,
    /// Create a new file
    CreateNewFile
  };
  /**
   * @return @c ReplaceMode that decides what parts of the active document should be replaced by the
   *         @c STDOUT of the @c command() execution.
   */
  ReplaceMode replaceMode() const;
  /**
   * Sets the @c ReplaceMode that decides what parts of the active document should be replaced by the
   * @c STDOUT of the @c command() execution.
   */
  void setReplaceMode( ReplaceMode mode );

  enum InputMode {
    /// Nothing gets streamed to the @c STDIN of the external script.
    InputNone,
    /// Current selection gets streamed into the @c STDIN of the external script.
    InputSelection,
    /// The whole contents of the active document get streamed into the @c STDIN of the external script.
    InputDocument
  };
  /**
   * @return @c InputMode that decides what parts of the active document should be streamded into
   *         the @c STDIN of the external script.
   */
  InputMode inputMode() const;
  /**
   * Sets the @c InputMode that decides what parts of the active document should be streamded into
   * the @c STDIN of the external script.
   */
  void setInputMode( InputMode mode );

  /**
   * Action to trigger insertion of this snippet.
   */
  KAction* action();

  /**
   * @return True when this command should have its output shown, false otherwise.
   */
  bool showOutput() const;
  /**
   * Set @p show to true when the output of this command shout be shown, false otherwise.
   */
  void setShowOutput( bool show );

  ///TODO: custom icon
  ///TODO: mimetype / language filter
  ///TODO: kate commandline integration
  ///TODO: filter for local/remote files

  /**
   * Saves this item after changes.
   */
  void save() const;
private:
  QString m_command;
  SaveMode m_saveMode;
  ReplaceMode m_replaceMode;
  InputMode m_inputMode;
  KAction* m_action;
  bool m_showOutput;
};

Q_DECLARE_METATYPE(ExternalScriptItem*)

#endif // EXTERNALSCRIPTITEM_H

// kate: indent-mode cstyle; space-indent on; indent-width 2; replace-tabs on; 
