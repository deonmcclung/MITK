/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center, 
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without 
even the implied warranty of MERCHANTABILITY or FITNESS FOR 
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryIPreferences.h>
#include <berryIPreferencesService.h>
#include <berryIBerryPreferences.h>

// Qmitk
#include "CommandLineModulesView.h"
#include "CommandLineModulesPreferencesPage.h"

// Qt
#include <QMessageBox>
#include <QScrollArea>
#include <QFile>
#include <QFileDialog>
#include <QFileInfoList>
#include <QDir>
#include <QBuffer>
#include <QtUiTools/QUiLoader>
#include <QByteArray>
#include <QHBoxLayout>
#include <QAction>
#include <QDebug>

// CTK
#include <ctkCmdLineModuleManager.h>
#include <ctkCmdLineModuleDirectoryWatcher.h>
#include <ctkCmdLineModuleMenuFactoryQtGui.h>
#include <ctkCmdLineModuleInstance.h>
#include <ctkCmdLineModuleDescription.h>
#include <ctkCmdLineModuleInstanceFactoryQtGui.h>
#include <ctkCmdLineModuleXmlValidator.h>
#include <ctkCmdLineModuleDefaultPathBuilder.h>

const std::string CommandLineModulesView::VIEW_ID = "org.mitk.gui.qt.cli";

CommandLineModulesView::CommandLineModulesView()
: m_Controls(NULL)
, m_Parent(NULL)
, m_ModuleManager(NULL)
, m_DirectoryWatcher(NULL)
, m_MenuFactory(NULL)
, m_TemporaryDirectoryName("")
{
  m_MapTabToModuleInstance.clear();
  m_ModuleManager = new ctkCmdLineModuleManager(new ctkCmdLineModuleInstanceFactoryQtGui());
  m_DirectoryWatcher = new ctkCmdLineModuleDirectoryWatcher(m_ModuleManager);
  m_MenuFactory = new ctkCmdLineModuleMenuFactoryQtGui();
}

CommandLineModulesView::~CommandLineModulesView()
{
  if (m_ModuleManager != NULL)
  {
    delete m_ModuleManager;
  }

  if (m_DirectoryWatcher != NULL)
  {
    delete m_DirectoryWatcher;
  }

  if (m_MenuFactory != NULL)
  {
    delete m_MenuFactory;
  }
}

void CommandLineModulesView::SetFocus()
{
  this->m_Controls->m_ComboBox->setFocus();
}

void CommandLineModulesView::CreateQtPartControl( QWidget *parent )
{
  if (!m_Controls)
  {
    // We create CommandLineModulesViewControls, which derives from the Qt generated class.
    m_Controls = new CommandLineModulesViewControls(parent);

    // This connect must come before we update the preferences for the first time.
    connect(this->m_ModuleManager, SIGNAL(moduleAdded(ctkCmdLineModuleReference)), this, SLOT(OnModulesChanged()));
    connect(this->m_ModuleManager, SIGNAL(moduleRemoved(ctkCmdLineModuleReference)), this, SLOT(OnModulesChanged()));

    // Loads the preferences like directory settings into member variables.
    this->RetrievePreferenceValues();

    // Connect signals to slots after we have set up GUI.
    connect(this->m_Controls->m_RunButton, SIGNAL(pressed()), this, SLOT(OnRunButtonPressed()));
    connect(this->m_Controls->m_StopButton, SIGNAL(pressed()), this, SLOT(OnStopButtonPressed()));
    connect(this->m_Controls->m_RestoreDefaults, SIGNAL(pressed()), this, SLOT(OnRestoreDefaultsButtonPressed()));
    connect(this->m_Controls->m_ComboBox, SIGNAL(actionChanged(QAction*)), this, SLOT(OnActionChanged(QAction*)));
  }
}

void CommandLineModulesView::RetrievePreferenceValues()
{
  berry::IPreferencesService::Pointer prefService
      = berry::Platform::GetServiceRegistry()
      .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  assert( prefService );

  std::string id = "/" + CommandLineModulesView::VIEW_ID;
  berry::IBerryPreferences::Pointer prefs
      = (prefService->GetSystemPreferences()->Node(id))
        .Cast<berry::IBerryPreferences>();

  assert( prefs );

  // Get the flag for debug output, useful when parsing all the XML.
  bool debugOutputBefore = m_DebugOutput;
  m_DebugOutput = prefs->GetBool(CommandLineModulesPreferencesPage::DEBUG_OUTPUT_NODE_NAME, false);
  m_DirectoryWatcher->setDebug(m_DebugOutput);

  // Not yet in use, will be used for ... er... temporary stuff.
  m_TemporaryDirectoryName = QString::fromStdString(prefs->Get(CommandLineModulesPreferencesPage::TEMPORARY_DIRECTORY_NODE_NAME, ""));

  // Get some default application paths.
  // Here we can use the preferences to set up the builder, before asking him for the paths to scan.
  ctkCmdLineModuleDefaultPathBuilder builder;
  QStringList defaultPaths = builder.build();

  // We get additional paths from preferences.
  QString pathString = QString::fromStdString(prefs->Get(CommandLineModulesPreferencesPage::MODULE_DIRECTORIES_NODE_NAME, ""));
  QStringList additionalPaths = pathString.split(";", QString::SkipEmptyParts);

  // Combine the sets of paths.
  QStringList totalPaths;
  totalPaths << defaultPaths;
  totalPaths << additionalPaths;

  // OnPreferencesChanged can be called for each preference in a dialog box, so
  // when you hit "OK", it is called repeatedly, whereas we want to only call this once,
  // so I am checking if the list of directory names has changed, and whether the debug flag has changed.
  if (this->m_DirectoryWatcher->directories() != totalPaths || (debugOutputBefore != m_DebugOutput))
  {
    // This should update the directory watcher, which should sort out if any new modules have been loaded
    // and if so, should signal ModulesChanged, which is caught by this class, and re-build the GUI.
    m_DirectoryWatcher->setDirectories(totalPaths);
  }
}

void CommandLineModulesView::OnPreferencesChanged(const berry::IBerryPreferences* /*prefs*/)
{
  // Loads the preferences into member variables.
  this->RetrievePreferenceValues();
}

void CommandLineModulesView::OnModulesChanged()
{
  QList<ctkCmdLineModuleReference> refs = this->m_ModuleManager->moduleReferences();
  QMenu *menu = m_MenuFactory->create(refs);
  this->m_Controls->m_ComboBox->setMenu(menu);
}

void CommandLineModulesView::AddModuleTab(const ctkCmdLineModuleReference& moduleRef)
{
  ctkCmdLineModuleInstance* moduleInstance = m_ModuleManager->createModuleInstance(moduleRef);
  if (!moduleInstance) return;

  QObject* guiHandle = moduleInstance->guiHandle();
  QWidget* widget = qobject_cast<QWidget*>(guiHandle);

  int tabIndex = m_Controls->m_TabWidget->addTab(widget, moduleRef.description().title());
  m_Controls->m_TabWidget->setCurrentIndex(tabIndex);
  m_MapTabToModuleInstance[tabIndex] = moduleInstance;

  m_Controls->m_HelpBrowser->clear();
  m_Controls->m_HelpBrowser->setPlainText(moduleRef.description().description());
  m_Controls->m_AboutBrowser->clear();
  m_Controls->m_AboutBrowser->setPlainText(moduleRef.description().acknowledgements());
}

void CommandLineModulesView::OnFutureFinished()
{
  qDebug() << "*** Future finished ***";
}

void CommandLineModulesView::OnActionChanged(QAction* action)
{
  ctkCmdLineModuleReference ref = this->GetReferenceByIdentifier(action->text());
  if (ref)
  {
    this->AddModuleTab(ref);
  }
}

ctkCmdLineModuleReference CommandLineModulesView::GetReferenceByIdentifier(QString identifier)
{
  ctkCmdLineModuleReference result;

  QList<ctkCmdLineModuleReference> references = this->m_ModuleManager->moduleReferences();

  ctkCmdLineModuleReference ref;
  foreach(ref, references)
  {
    if (ref.description().title() == identifier)
    {
      result = ref;
    }
  }

  return result;
}

void CommandLineModulesView::OnRunButtonPressed()
{
  qDebug() << "Creating module command line...";

  ctkCmdLineModuleInstance* moduleInstance = m_MapTabToModuleInstance[m_Controls->m_TabWidget->currentIndex()];
  if (!moduleInstance)
  {
    qWarning() << "Invalid module instance";
    return;
  }

  qDebug() << "Launching module command line...";

  qDebug() << "Launched module command line...";
}

void CommandLineModulesView::OnStopButtonPressed()
{
  qDebug() << "Stopping module command line...";


  qDebug() << "Stopped module command line...";
}

void CommandLineModulesView::OnRestoreDefaultsButtonPressed()
{

}
