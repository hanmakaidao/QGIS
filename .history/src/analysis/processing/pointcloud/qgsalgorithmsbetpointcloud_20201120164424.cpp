/***************************************************************************
                         qgsalgorithmsbetpointcloud.cpp
                         ---------------------
    begin                : May 2020
    copyright            : (C) 2020 by Clemens Raffler
    email                : clemens dot raffler at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmsbetpointcloud.h"
#include "qgsrasterfilewriter.h"
#include "qgsstringutils.h"
#include "random"
#include "limits"

///@cond PRIVATE

//
// QgsPointCloudGeoReferenceAlgorithmBase
//
QString QgsPointCloudGeoReferenceAlgorithmBase::group() const
{
  return QObject::tr( "Point Cloud" );
}

QString QgsPointCloudGeoReferenceAlgorithmBase::groupId() const
{
  return QStringLiteral( "pointcloud" );
}

void QgsPointCloudGeoReferenceAlgorithmBase::initAlgorithm( const QVariantMap & )
{
 // const QString &name, const QString &description = QString(), Behavior behavior = File, const QString &extension = QString(), const QVariant &defaultValue = QVariant(),
 //                               bool optional = false, const QString &fileFilter = QString()
  addParameter( new QgsProcessingParameterFile( QStringLiteral( "Input point cloud file" ),QStringLiteral( "point  cloud with out geo-correction,regal production" ),Behavior::file,QObject::tr( "las;laz")) );
  addParameter( new QgsProcessingParameterFile( QStringLiteral( "Sbet File" ),QStringLiteral( "Sbel file ,pos information" ),Behavior::file) );
  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "TARGET_CRS" ), QObject::tr( "Target CRS" ), QStringLiteral( "ProjectCrs" ) ) );
  //addParameter( new QgsProcessingParameterNumber( QStringLiteral( "PIXEL_SIZE" ), QObject::tr( "Pixel size" ),
  //              QgsProcessingParameterNumber::Double, 1, false, 0.01 ) );

  //add specific parameters
  addAlgorithmParams();

  addParameter( new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Output point cloud" ) ) );
}

bool QgsPointCloudGeoReferenceAlgorithmBase::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );
  mCrs = parameterAsCrs( parameters, QStringLiteral( "TARGET_CRS" ), context );
  //mExtent = parameterAsExtent( parameters, QStringLiteral( "EXTENT" ), context, mCrs );
  //mPixelSize = parameterAsDouble( parameters, QStringLiteral( "PIXEL_SIZE" ), context );

  return true;
}

QVariantMap QgsPointCloudGeoReferenceAlgorithmBase::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{

  return QVariantMap();
}

//
//QgsPointCloudGeoRefWithSbetAlgorithm
//
QString QgsPointCloudGeoRefWithSbetAlgorithm::name() const
{
  return QStringLiteral( "GeoReferencePointCloud" );
}

QString QgsPointCloudGeoRefWithSbetAlgorithm::displayName() const
{
  return QObject::tr( "geo-correct Point Cloud data file)" );
}

QStringList QgsPointCloudGeoRefWithSbetAlgorithm::tags() const
{
  return QObject::tr( "point cloud,geospatial,sbet" ).split( ',' );
}

QString QgsPointCloudGeoRefWithSbetAlgorithm::shortHelpString() const
{
  return QObject::tr( "使用姿轨曲线对点云进行地理矫正" );
}

QgsPointCloudGeoRefWithSbetAlgorithm *QgsPointCloudGeoRefWithSbetAlgorithm::createInstance() const
{
  return new QgsPointCloudGeoRefWithSbetAlgorithm();
}

void QgsPointCloudGeoRefWithSbetAlgorithm::addAlgorithmParams()
{
}

bool QgsPointCloudGeoRefWithSbetAlgorithm::getPcdInfo( const QVariantMap &parameters, QgsProcessingContext &context )
{
  return true;
}

///@endcond
