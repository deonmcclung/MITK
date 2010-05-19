/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-08 11:04:08 +0200 (Mi, 08 Jul 2009) $
Version:   $Revision: 18029 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkPlanarCircle.h"
#include "mitkGeometry2D.h"


mitk::PlanarCircle::PlanarCircle()
: FEATURE_ID_RADIUS( this->AddFeature( "Radius", "mm" ) ),
  FEATURE_ID_DIAMETER( this->AddFeature( "Diameter", "mm" ) ),
  FEATURE_ID_AREA( this->AddFeature( "Area", "mm^2" ) )
{
  // Circle has two control points
  this->ResetNumberOfControlPoints( 2 );

  m_PolyLines->InsertElement( 0, VertexContainerType::New());
}


mitk::PlanarCircle::~PlanarCircle()
{
}


//void mitk::PlanarCircle::Initialize()
//{
//  // Default initialization of circle control points
//
//  mitk::Geometry2D *geometry2D = 
//    dynamic_cast< mitk::Geometry2D * >( this->GetGeometry( 0 ) );
//
//  if ( geometry2D == NULL )
//  {
//    MITK_ERROR << "Missing Geometry2D for PlanarCircle";
//    return;
//  }
//
//  mitk::ScalarType width = geometry2D->GetBounds()[1];
//  mitk::ScalarType height = geometry2D->GetBounds()[3];
//  
//  mitk::Point2D &centerPoint = m_ControlPoints->ElementAt( 0 );
//  mitk::Point2D &boundaryPoint = m_ControlPoints->ElementAt( 1 );
//
//  centerPoint[0] = width / 2.0;
//  centerPoint[1] = height / 2.0;
//
//  boundaryPoint[0] = centerPoint[0] + 20.0;
//  boundaryPoint[1] = centerPoint[1];
//}

bool mitk::PlanarCircle::SetControlPoint( unsigned int index, const Point2D &point, bool /*createIfDoesNotExist*/ )
{
  // moving center point
  if(index == 0)
  {
    const Point2D &centerPoint = m_ControlPoints->ElementAt( 0 );
    Point2D boundaryPoint = m_ControlPoints->ElementAt( 1 );
    vnl_vector<float> vec = (point.GetVnlVector() - centerPoint.GetVnlVector());

    boundaryPoint[0] += vec[0];
    boundaryPoint[1] += vec[1];
    m_ControlPoints->InsertElement( 0, point );
    m_ControlPoints->InsertElement( 1, boundaryPoint );
  }
  else if ( index == 1 )
  {
    m_ControlPoints->InsertElement( index, point );
    return true;
  }
  return false;
}

void mitk::PlanarCircle::GeneratePolyLine()
{
  // TODO: start circle at specified boundary point...

  const Point2D &centerPoint = m_ControlPoints->ElementAt( 0 );
  const Point2D &boundaryPoint = m_ControlPoints->ElementAt( 1 );

  double radius = centerPoint.EuclideanDistanceTo( boundaryPoint );

  // Generate poly-line with 64 segments
  m_PolyLines->ElementAt( 0 )->Reserve( 64 );
  for ( int t = 0; t < 64; ++t )
  {
    double alpha = (double) t * vnl_math::pi / 32.0;

    m_PolyLines->ElementAt( 0 )->ElementAt( t )[0] = centerPoint[0] + radius * cos( alpha );
    m_PolyLines->ElementAt( 0 )->ElementAt( t )[1] = centerPoint[1] + radius * sin( alpha );
  }
}

void mitk::PlanarCircle::GenerateHelperPolyLine(double /*mmPerDisplayUnit*/, unsigned int /*displayHeight*/)
{
  // A circle does not require a helper object
}

void mitk::PlanarCircle::EvaluateFeaturesInternal()
{
  // Calculate circle radius and area
  const Point3D &p0 = this->GetWorldControlPoint( 0 );
  const Point3D &p1 = this->GetWorldControlPoint( 1 );

  double radius = p0.EuclideanDistanceTo( p1 );
  double area = vnl_math::pi * radius * radius;

  this->SetQuantity( FEATURE_ID_RADIUS, radius );
  this->SetQuantity( FEATURE_ID_DIAMETER, 2*radius );
  this->SetQuantity( FEATURE_ID_AREA, area );
}


void mitk::PlanarCircle::PrintSelf( std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf( os, indent );
}