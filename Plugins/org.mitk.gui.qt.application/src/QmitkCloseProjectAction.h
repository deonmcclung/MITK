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

#ifndef QmitkCloseProjectAction_H_
#define QmitkCloseProjectAction_H_

#ifdef __MINGW32__
// We need to include winbase.h here in order to declare
// atomic intrinsics like InterlockedIncrement correctly.
// Otherwhise, they would be declared wrong within qatomic_windows.h .
#include <windows.h>
#endif

#include <QAction>

#include <org_mitk_gui_qt_application_Export.h>

#include <berryIWorkbenchWindow.h>

/**
 * \ingroup org_mitk_gui_qt_application
 */
class MITK_QT_APP QmitkCloseProjectAction : public QAction
{
  Q_OBJECT

public:
  QmitkCloseProjectAction(berry::IWorkbenchWindow::Pointer window);
  QmitkCloseProjectAction(const QIcon & icon, berry::IWorkbenchWindow::Pointer window);
protected slots:
  void Run();

private:
  void init(berry::IWorkbenchWindow::Pointer window);
  berry::IWorkbenchWindow::Pointer m_Window;
};
#endif /*QmitkCloseProjectAction_H_*/
