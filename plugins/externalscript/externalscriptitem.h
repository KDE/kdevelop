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

#ifndef KDEVPLATFORM_PLUGIN_EXTERNALSCRIPTITEM_H
#define KDEVPLATFORM_PLUGIN_EXTERNALSCRIPTITEM_H

#include <QStandardItemModel>

class QAction;

/**
 * NOTE: use @c text() and @c setText() to define the label/name of the external script.
 */
class ExternalScriptItem : public QStandardItem
{
public:
  ExternalScriptItem();

  /**
   * @return The command to execute.
   */
  QString command() const;
  /**
   * Sets the command to execute.
   */
  void setCommand( const QString& command );

  /**
   * @return The working directory where to execute the command.
   *         If this is empty (default), it should be derived from the active document.
   */
  QString workingDirectory() const;
  
  /**
   * Specify the working directory where the command should be executed
   */
  void setWorkingDirectory( const QString& workingDirectory );
  
  /**
   * Whether placeholders like %b etc. in the command should be substituted. Default is true.
   * */
  bool performParameterReplacement() const;
  
  /**
   * Set whether placeholders like %b etc. in the command should be substituted. Default is true.
   * */
  void setPerformParameterReplacement(bool perform);
  
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

  /**
   * @return what type of filter should be applied to the execution of the external script
   **/
  int filterMode() const;

  /**
   * Sets the filtering mode 
   **/
  void setFilterMode( int mode );

  /// Defines what should be done with the @c STDOUT of a script run.
  enum OutputMode {
    /// Ignore output and do nothing.
    OutputNone,
    /// Output gets inserted at the cursor position of the current document.
    OutputInsertAtCursor,
    /// Current selection gets replaced in the active document.
    /// If no selection exists, the output will get inserted at the
    /// current cursor position in the active document view.
    OutputReplaceSelectionOrInsertAtCursor,
    /// Current selection gets replaced in the active document.
    /// If no selection exists, the whole document gets replaced.
    OutputReplaceSelectionOrDocument,
    /// The whole contents of the active document gets replaced.
    OutputReplaceDocument,
    /// Create a new file from the output.
    OutputCreateNewFile
  };
  /**
   * @return @c OutputMode that decides what parts of the active document should be replaced by the
   *         @c STDOUT of the @c command() execution.
   */
  OutputMode outputMode() const;
  /**
   * Sets the @c OutputMode that decides what parts of the active document should be replaced by the
   * @c STDOUT of the @c command() execution.
   */
  void setOutputMode( OutputMode mode );

  /// Defines what should be done with the @c STDERR of a script run.
  enum ErrorMode {
    /// Ignore errors and do nothing.
    ErrorNone,
    /// Merge with @c STDOUT and use @c OutputMode.
    ErrorMergeOutput,
    /// Errors get inserted at the cursor position of the current document.
    ErrorInsertAtCursor,
    /// Current selection gets replaced in the active document.
    /// If no selection exists, the output will get inserted at the
    /// current cursor position in the active document view.
    ErrorReplaceSelectionOrInsertAtCursor,
    /// Current selection gets replaced in the active document.
    /// If no selection exists, the whole document gets replaced.
    ErrorReplaceSelectionOrDocument,
    /// The whole contents of the active document gets replaced.
    ErrorReplaceDocument,
    /// Create a new file from the errors.
    ErrorCreateNewFile
  };

  /**
   * @return @c ErrorMode that decides what parts of the active document should be replaced by the
   *         @c STDERR of the @c command() execution.
   */
  ErrorMode errorMode() const;
  /**
   * Sets the @c ErrorMode that decides what parts of the active document should be replaced by the
   * @c STDERR of the @c command() execution.
   */
  void setErrorMode( ErrorMode mode );

  enum InputMode {
    /// Nothing gets streamed to the @c STDIN of the external script.
    InputNone,
    /// Current selection gets streamed into the @c STDIN of the external script.
    /// If no selection exists, nothing gets streamed.
    InputSelectionOrNone,
    /// Current selection gets streamed into the @c STDIN of the external script.
    /// If no selection exists, the whole document gets streamed.
    InputSelectionOrDocument,
    /// The whole contents of the active document get streamed into the @c STDIN of the external script.
    InputDocument,
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
  QAction* action();

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
  QString m_workingDirectory;
  SaveMode m_saveMode;
  OutputMode m_outputMode;
  ErrorMode m_errorMode;
  InputMode m_inputMode;
  QAction* m_action;
  bool m_showOutput;
  int m_filterMode;
  bool m_performReplacements;
};

Q_DECLARE_METATYPE(ExternalScriptItem*)

#endif // KDEVPLATFORM_PLUGIN_EXTERNALSCRIPTITEM_H

// kate: indent-mode cstyle; space-indent on; indent-width 2; replace-tabs on; 
