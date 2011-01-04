/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkImage.h"
#include "mitkPlaneGeometry.h"
#include "mitkTimeSlicedGeometry.h"
#include "mitkSlicedGeometry3D.h"
#include "mitkTestingMacros.h"

#include <vnl/vnl_quaternion.h>
#include <vnl/vnl_quaternion.txx>

#include <fstream>

void mitkSlicedGeometry3D_ChangeImageGeometryConsideringOriginOffset_Test()
{ 
  //Tests for Offset
  MITK_TEST_OUTPUT( << "====== NOW RUNNING: Tests for pixel-center-based offset concerns ========");  


  // create a SlicedGeometry3D
  mitk::SlicedGeometry3D::Pointer slicedGeo3D=mitk::SlicedGeometry3D::New();
  int num_slices = 5;
  slicedGeo3D->Initialize(num_slices); // 5 slices
  mitk::Point3D newOrigin;
  newOrigin[0] = 91.3;
  newOrigin[1] = -13.3;
  newOrigin[2] = 0;
  slicedGeo3D->SetOrigin(newOrigin);
  mitk::Vector3D newSpacing;
  newSpacing[0] = 1.0f;
  newSpacing[1] = 0.9f;
  newSpacing[2] = 0.3f;
  slicedGeo3D->SetSpacing(newSpacing);
  // create subslices as well  
  for (int i=0; i < num_slices; i++)
  {
    mitk::Geometry2D::Pointer geo2d = mitk::Geometry2D::New();
    geo2d->Initialize();
    slicedGeo3D->SetGeometry2D(geo2d,i);
  } 

  // now run tests
  
  MITK_TEST_OUTPUT( << "Testing whether slicedGeo3D->GetImageGeometry() is false by default");  
  MITK_TEST_CONDITION_REQUIRED( slicedGeo3D->GetImageGeometry()==false, "");
  MITK_TEST_OUTPUT( << "Testing whether first and last geometry in the SlicedGeometry3D have GetImageGeometry()==false by default");
  mitk::Geometry2D* subSliceGeo2D_first = slicedGeo3D->GetGeometry2D(0);
  mitk::Geometry2D* subSliceGeo2D_last = slicedGeo3D->GetGeometry2D(num_slices-1);
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo2D_first->GetImageGeometry()==false, "");
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo2D_last->GetImageGeometry()==false, "");

  // Save CornerPoints of SlicedGeometry3D, the included first Planegeometry, and the included last Planegeometry for later..
  mitk::Point3D OriginofSlicedGeometry( slicedGeo3D->GetOrigin() ); //copy constructor
  mitk::Point3D CornerPoint0SlicedGeometry = slicedGeo3D->GetCornerPoint(0);
  mitk::Point3D CornerPoint1FirstGeometry = subSliceGeo2D_first->GetCornerPoint(1);
  mitk::Point3D CornerPoint2LastGeometry = subSliceGeo2D_last->GetCornerPoint(2);

  MITK_TEST_OUTPUT( << "Calling slicedGeo3D->ChangeImageGeometryConsideringOriginOffset(true)");
  slicedGeo3D->ChangeImageGeometryConsideringOriginOffset(true);
  MITK_TEST_OUTPUT( << "Testing whether slicedGeo3D->GetImageGeometry() is now true");
  MITK_TEST_CONDITION_REQUIRED( slicedGeo3D->GetImageGeometry()==true, "");
  MITK_TEST_OUTPUT( << "Testing whether first and last geometry in the SlicedGeometry3D have GetImageGeometry()==true now");
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo2D_first->GetImageGeometry()==true, "");
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo2D_last->GetImageGeometry()==true, "");  

  MITK_TEST_OUTPUT( << "Testing wether offset has been added to cornerPoints of geometry");

  // Manually adding Offset.
  CornerPoint1FirstGeometry[0] += (subSliceGeo2D_first->GetSpacing()[0]) / 2;
  CornerPoint1FirstGeometry[1] += (subSliceGeo2D_first->GetSpacing()[1]) / 2;
  CornerPoint1FirstGeometry[2] += (subSliceGeo2D_first->GetSpacing()[2]) / 2;  
  CornerPoint2LastGeometry[0] += (subSliceGeo2D_last->GetSpacing()[0]) / 2;
  CornerPoint2LastGeometry[1] += (subSliceGeo2D_last->GetSpacing()[1]) / 2;
  CornerPoint2LastGeometry[2] += (subSliceGeo2D_last->GetSpacing()[2]) / 2;
  CornerPoint0SlicedGeometry[0] += (slicedGeo3D->GetSpacing()[0]) / 2;
  CornerPoint0SlicedGeometry[1] += (slicedGeo3D->GetSpacing()[1]) / 2;
  CornerPoint0SlicedGeometry[2] += (slicedGeo3D->GetSpacing()[2]) / 2;
  OriginofSlicedGeometry[0] += (slicedGeo3D->GetSpacing()[0]) / 2;
  OriginofSlicedGeometry[1] += (slicedGeo3D->GetSpacing()[1]) / 2;
  OriginofSlicedGeometry[2] += (slicedGeo3D->GetSpacing()[2]) / 2;

  MITK_TEST_CONDITION_REQUIRED( subSliceGeo2D_first->GetCornerPoint(1)==CornerPoint1FirstGeometry, "");
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo2D_last->GetCornerPoint(2)==CornerPoint2LastGeometry, "");
  MITK_TEST_CONDITION_REQUIRED( slicedGeo3D->GetCornerPoint(0)==CornerPoint0SlicedGeometry, "");  
  MITK_TEST_CONDITION_REQUIRED( slicedGeo3D->GetOrigin()==OriginofSlicedGeometry, "");


  MITK_TEST_OUTPUT( << "Calling slicedGeo3D->ChangeImageGeometryConsideringOriginOffset(false)");
  slicedGeo3D->ChangeImageGeometryConsideringOriginOffset(false);
  MITK_TEST_OUTPUT( << "Testing whether slicedGeo3D->GetImageGeometry() is now false");
  MITK_TEST_CONDITION_REQUIRED( slicedGeo3D->GetImageGeometry()==false, "");
  MITK_TEST_OUTPUT( << "Testing whether first and last geometry in the SlicedGeometry3D have GetImageGeometry()==false now");
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo2D_first->GetImageGeometry()==false, "");
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo2D_last->GetImageGeometry()==false, "");  

  MITK_TEST_OUTPUT( << "Testing wether offset has been added to cornerPoints of geometry");

  // Manually substracting Offset.
  CornerPoint1FirstGeometry[0] -= (subSliceGeo2D_first->GetSpacing()[0]) / 2;
  CornerPoint1FirstGeometry[1] -= (subSliceGeo2D_first->GetSpacing()[1]) / 2;
  CornerPoint1FirstGeometry[2] -= (subSliceGeo2D_first->GetSpacing()[2]) / 2;  
  CornerPoint2LastGeometry[0] -= (subSliceGeo2D_last->GetSpacing()[0]) / 2;
  CornerPoint2LastGeometry[1] -= (subSliceGeo2D_last->GetSpacing()[1]) / 2;
  CornerPoint2LastGeometry[2] -= (subSliceGeo2D_last->GetSpacing()[2]) / 2;
  CornerPoint0SlicedGeometry[0] -= (slicedGeo3D->GetSpacing()[0]) / 2;
  CornerPoint0SlicedGeometry[1] -= (slicedGeo3D->GetSpacing()[1]) / 2;
  CornerPoint0SlicedGeometry[2] -= (slicedGeo3D->GetSpacing()[2]) / 2;
  OriginofSlicedGeometry[0] -= (slicedGeo3D->GetSpacing()[0]) / 2;
  OriginofSlicedGeometry[1] -= (slicedGeo3D->GetSpacing()[1]) / 2;
  OriginofSlicedGeometry[2] -= (slicedGeo3D->GetSpacing()[2]) / 2;

  MITK_TEST_CONDITION_REQUIRED( subSliceGeo2D_first->GetCornerPoint(1)==CornerPoint1FirstGeometry, "");
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo2D_last->GetCornerPoint(2)==CornerPoint2LastGeometry, "");
  MITK_TEST_CONDITION_REQUIRED( slicedGeo3D->GetCornerPoint(0)==CornerPoint0SlicedGeometry, "");  
  MITK_TEST_CONDITION_REQUIRED( slicedGeo3D->GetOrigin()==OriginofSlicedGeometry, "");
  
  MITK_TEST_OUTPUT( << "ALL SUCCESSFULLY!");
}

int mitkSlicedGeometry3DTest(int /*argc*/, char* /*argv*/[])
{
  mitk::PlaneGeometry::Pointer planegeometry1 = mitk::PlaneGeometry::New();
 
  mitk::Point3D origin;
  mitk::Vector3D right, bottom, normal;
  mitk::ScalarType width, height;
  mitk::ScalarType widthInMM, heightInMM, thicknessInMM;

  width  = 100;    widthInMM  = width;
  height = 200;    heightInMM = height;
  thicknessInMM = 3.5;
  mitk::FillVector3D(origin, 4.5,              7.3, 11.2);
  mitk::FillVector3D(right,  widthInMM,          0, 0);
  mitk::FillVector3D(bottom,         0, heightInMM, 0);
  mitk::FillVector3D(normal,         0,          0, thicknessInMM);

  std::cout << "Initializing planegeometry1 by InitializeStandardPlane(rightVector, downVector, spacing = NULL): "<<std::endl;
  planegeometry1->InitializeStandardPlane(right.Get_vnl_vector(), bottom.Get_vnl_vector());

  std::cout << "Setting planegeometry2 to a cloned version of planegeometry1: "<<std::endl;
  mitk::PlaneGeometry::Pointer planegeometry2;
  planegeometry2 = dynamic_cast<mitk::PlaneGeometry*>(planegeometry1->Clone().GetPointer());;

  std::cout << "Changing the IndexToWorldTransform of planegeometry2 to a rotated version by SetIndexToWorldTransform() (keep origin): "<<std::endl;
  mitk::AffineTransform3D::Pointer transform = mitk::AffineTransform3D::New();
  mitk::AffineTransform3D::MatrixType::InternalMatrixType vnlmatrix;
  vnlmatrix = planegeometry2->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix();
  mitk::VnlVector axis(3);
  mitk::FillVector3D(axis, 1.0, 1.0, 1.0); axis.normalize();
  vnl_quaternion<mitk::ScalarType> rotation(axis, 0.123);
  vnlmatrix = rotation.rotation_matrix_transpose()*vnlmatrix;
  mitk::Matrix3D matrix;
  matrix = vnlmatrix;
  transform->SetMatrix(matrix);
  transform->SetOffset(planegeometry2->GetIndexToWorldTransform()->GetOffset());
  
  right.Set_vnl_vector( rotation.rotation_matrix_transpose()*right.Get_vnl_vector() );
  bottom.Set_vnl_vector(rotation.rotation_matrix_transpose()*bottom.Get_vnl_vector());
  normal.Set_vnl_vector(rotation.rotation_matrix_transpose()*normal.Get_vnl_vector());
  planegeometry2->SetIndexToWorldTransform(transform);


  std::cout << "Setting planegeometry3 to the backside of planegeometry2: " <<std::endl;
  mitk::PlaneGeometry::Pointer planegeometry3 = mitk::PlaneGeometry::New();
  planegeometry3->InitializeStandardPlane(planegeometry2, mitk::PlaneGeometry::Transversal, 0, false);


  std::cout << "Testing SlicedGeometry3D::InitializeEvenlySpaced(planegeometry3, zSpacing = 1, slices = 5, flipped = false): " <<std::endl;
  mitk::SlicedGeometry3D::Pointer slicedWorldGeometry=mitk::SlicedGeometry3D::New();
  unsigned int numSlices = 5;
  slicedWorldGeometry->InitializeEvenlySpaced(planegeometry3, 1, numSlices, false);

  std::cout << "Testing availability and type (PlaneGeometry) of first geometry in the SlicedGeometry3D: ";
  mitk::PlaneGeometry* accessedplanegeometry3 = dynamic_cast<mitk::PlaneGeometry*>(slicedWorldGeometry->GetGeometry2D(0));
  if(accessedplanegeometry3==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing whether the first geometry in the SlicedGeometry3D is identical to planegeometry3 by axis comparison and origin: "<<std::endl;
  if((mitk::Equal(accessedplanegeometry3->GetAxisVector(0), planegeometry3->GetAxisVector(0))==false) || 
     (mitk::Equal(accessedplanegeometry3->GetAxisVector(1), planegeometry3->GetAxisVector(1))==false) || 
     (mitk::Equal(accessedplanegeometry3->GetAxisVector(2), planegeometry3->GetAxisVector(2))==false) ||
     (mitk::Equal(accessedplanegeometry3->GetOrigin(), planegeometry3->GetOrigin())==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing availability and type (PlaneGeometry) of the last geometry in the SlicedGeometry3D: ";
  mitk::PlaneGeometry* accessedplanegeometry3last = dynamic_cast<mitk::PlaneGeometry*>(slicedWorldGeometry->GetGeometry2D(numSlices-1));
  mitk::Point3D origin3last; origin3last = planegeometry3->GetOrigin()+slicedWorldGeometry->GetDirectionVector()*(numSlices-1);
  if(accessedplanegeometry3last==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing whether the last geometry in the SlicedGeometry3D is identical to planegeometry3 by axis comparison: "<<std::endl;
  if((mitk::Equal(accessedplanegeometry3last->GetAxisVector(0), planegeometry3->GetAxisVector(0))==false) || 
     (mitk::Equal(accessedplanegeometry3last->GetAxisVector(1), planegeometry3->GetAxisVector(1))==false) || 
     (mitk::Equal(accessedplanegeometry3last->GetAxisVector(2), planegeometry3->GetAxisVector(2))==false) ||
     (mitk::Equal(accessedplanegeometry3last->GetOrigin(), origin3last)==false) ||
     (mitk::Equal(accessedplanegeometry3last->GetIndexToWorldTransform()->GetOffset(), origin3last.GetVectorFromOrigin())==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Again for first slice - Testing availability and type (PlaneGeometry) of first geometry in the SlicedGeometry3D: ";
  accessedplanegeometry3 = dynamic_cast<mitk::PlaneGeometry*>(slicedWorldGeometry->GetGeometry2D(0));
  if(accessedplanegeometry3==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Again for first slice - Testing whether the first geometry in the SlicedGeometry3D is identical to planegeometry3 by axis comparison and origin: "<<std::endl;
  if((mitk::Equal(accessedplanegeometry3->GetAxisVector(0), planegeometry3->GetAxisVector(0))==false) || 
     (mitk::Equal(accessedplanegeometry3->GetAxisVector(1), planegeometry3->GetAxisVector(1))==false) || 
     (mitk::Equal(accessedplanegeometry3->GetAxisVector(2), planegeometry3->GetAxisVector(2))==false) ||
     (mitk::Equal(accessedplanegeometry3->GetOrigin(), planegeometry3->GetOrigin())==false) ||
     (mitk::Equal(accessedplanegeometry3->GetIndexToWorldTransform()->GetOffset(), planegeometry3->GetOrigin().GetVectorFromOrigin())==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  mitkSlicedGeometry3D_ChangeImageGeometryConsideringOriginOffset_Test();

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
