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

#include "QmitkRegisterClasses.h"
#include "QmitkRenderWindow.h"

#include <mitkDataNodeFactory.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkImageWriteAccessor.h>
#include <mitkImageReadAccessor.h>
#include <mitkImageVtkMapper2D.h>

#include <itksys/SystemTools.hxx>
#include <QApplication>

#include <sys/time.h>
#include <unistd.h>

using namespace mitk;

// global container for test images
std::vector<Image::Pointer> static_Images;
Image::Pointer static_ContainerImage;
  
StandaloneDataStorage::Pointer static_DataStorage;

void LoadImages(int argc, char* argv[])
{
  for (int a = 1; a < argc; ++a)
  {
    MITK_INFO << "Loading file " << argv[a];

    DataNodeFactory::Pointer reader=DataNodeFactory::New();
    const char * filename = argv[a];
    try
    {
      reader->SetFileName(filename);
      reader->Update();
      //*************************************************************************
      // Part III: Put the data into the datastorage
      //*************************************************************************

      for (unsigned int outputIdx = 0; outputIdx < reader->GetNumberOfOutputs(); ++outputIdx)
      {
        // Add the node to the DataStorage
        Image::Pointer image = dynamic_cast<Image*>(reader->GetOutput(outputIdx)->GetData());
        if (image.IsNotNull())
        {
          MITK_INFO << "Image " << argv[a] << " [" << outputIdx << "]";
          //image->Print(std::cout);
          image->DisconnectPipeline();
          static_Images.push_back( image );
        }
      }
    }
    catch(...)
    {
      fprintf( stderr, "Could not open file %s \n\n", filename );
      exit(EXIT_FAILURE);
    }
  }

  int a = 2;
  MITK_INFO << "Loading file as container image";

  DataNodeFactory::Pointer reader=DataNodeFactory::New();
  const char * filename = argv[a];
  try
  {
    reader->SetFileName(filename);
    reader->Update();
    //*************************************************************************
    // Part III: Put the data into the datastorage
    //*************************************************************************

    // Add the node to the DataStorage
    Image::Pointer image = dynamic_cast<Image*>(reader->GetOutput()->GetData());
    if (image.IsNotNull())
    {
      MITK_INFO << "Image " << argv[a];
      image->DisconnectPipeline();
      static_ContainerImage = image;
    }
  }
  catch(...)
  {
    fprintf( stderr, "Could not open file %s \n\n", filename );
    exit(EXIT_FAILURE);
  }
}

void Time(QmitkRenderWindow* rw, void (*renderFunction)(QmitkRenderWindow*, Image*), int numberOfFrames, std::string title)
{
  std::cout << "Method '" << title << "':" << std::flush;

  int numberOfImages = static_Images.size();
  int currentImageIndex = 0;

  struct timeval start, end;
  gettimeofday(&start, NULL);

  for (int frame = 0; frame < numberOfFrames; ++frame)
  {
    Image* currentImage = static_Images[currentImageIndex++ % numberOfImages];

    renderFunction(rw, currentImage);

    if (frame%100==0) std::cout << "." << std::flush;
  }

  gettimeofday(&end, NULL);

  long seconds  = end.tv_sec  - start.tv_sec;
  long useconds = end.tv_usec - start.tv_usec;

  long mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;

  MITK_INFO << mtime << " milliseconds for " << numberOfFrames << "frames , i.e. " 
            << (float)numberOfFrames / ((float)mtime / 1000.0) << "fps or " 
            << (float)mtime/(float)numberOfFrames << "ms/frame";
}

void RenderNaive(QmitkRenderWindow*, Image* image)
{
  /*
    simplest, obviously slow solution:
     - clear data storage
     - create a new node with the current image
     - add the node
     - repaint
  */
  DataNode::Pointer node = DataNode::New();
  node->SetData( image );
  static_DataStorage->Remove( static_DataStorage->GetAll()->front() );
  static_DataStorage->Add( node );

  mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
}

void RenderSwapImage(QmitkRenderWindow*, Image* image)
{
  /*
    seemingly most simple solution:
     - use ONE node with the current image
     - with each frame call SetData(currentImage)
     - repaint
  */
  static DataNode::Pointer node = static_DataStorage->GetAll()->front(); // initialized only ONCE to the first node in data storage
  
  node->SetData( image );

  mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
}

void RenderMemCopyImage(QmitkRenderWindow*, Image* image)
{
  /*
    seemingly most simple solution:
     - use ONE node with the current image
     - with each frame call SetData(currentImage)
     - repaint
  */
  static ImageWriteAccessor writer( static_ContainerImage, static_ContainerImage->GetSliceData(0) );
  static unsigned int numberOfBytes = image->GetDimension(0)*image->GetDimension(1)*image->GetPixelType().GetSize();

  ImageReadAccessor reader( image, image->GetSliceData(0) );
  
  memcpy( writer.GetData(), reader.GetData(), numberOfBytes );
  static_ContainerImage->Modified(); // mark as modified, so the mapper will draw again

  mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
}


//##Documentation
//## @brief Load image (nrrd format) and display it in a 2D view
int main(int argc, char* argv[])
{
  QApplication qtapplication( argc, argv );

  if (argc < 2)
  {
    fprintf( stderr, "Usage:   %s [filename] \n\n", itksys::SystemTools::GetFilenameName(argv[0]).c_str() );
    return 1;
  }

  // Register Qmitk-dependent global instances TODO needed here?
  QmitkRegisterClasses();

  LoadImages(argc, argv);

  // Create a DataStorage
  static_DataStorage = StandaloneDataStorage::New();
  
  // Create a RenderWindow
  QmitkRenderWindow renderWindow;
  renderWindow.GetRenderer()->SetDataStorage(static_DataStorage);

  DataNode::Pointer node = DataNode::New();
  node->SetData( static_Images[0] );
  static_DataStorage->Add( node ); // just so that the loop does not crash

  //TimeSlicedGeometry::Pointer geo = static_DataStorage->ComputeBoundingGeometry3D(static_DataStorage->GetAll());
  //RenderingManager::GetInstance()->InitializeViews( geo );
  RenderingManager::GetInstance()->InitializeViews( node->GetData()->GetGeometry() );

  // Select a slice
  renderWindow.GetSliceNavigationController()->GetSlice()->SetPos(0);

  //*************************************************************************
  // Part V: Qt-specific initialization
  //*************************************************************************
  renderWindow.show();
  
  int rwWidth = static_ContainerImage->GetDimension(0);
  int rwHeight = static_ContainerImage->GetDimension(1);
  renderWindow.resize( rwWidth, rwHeight );

  qtapplication.processEvents();

//  Time(&renderWindow, RenderNaive, 200, "Naive");
  
  Time(&renderWindow, RenderSwapImage, 200, "Swapping images");
  
  static_DataStorage->Remove( static_DataStorage->GetAll()->front() );
  node = DataNode::New();
  node->SetData( static_ContainerImage ); // we need a SEPARATE container image for the next method
  static_DataStorage->Add( node );
  Time(&renderWindow, RenderMemCopyImage, 200, "MemCopy images");
 
  mitk::ImageVtkMapper2D::SetReslicingFlag(false);
  Time(&renderWindow, RenderMemCopyImage, 200, "MemCopy images w/o reslicing");
  
  exit(EXIT_SUCCESS);
}

