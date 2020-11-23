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
  return QObject::tr( "Generates a raster layer for given extent and cell size "
                      "filled with random values.\n"
                      "By default, the values will range between the minimum and "
                      "maximum value of the specified output raster type. This can "
                      "be overridden by using the advanced parameters for lower and "
                      "upper bound value. If the bounds have the same value or both "
                      "are zero (default) the algorithm will create random values in "
                      "the full value range of the chosen raster data type. "
                      "Choosing bounds outside the acceptable range of the output "
                      "raster type will abort the algorithm." );
}

QgsPointCloudGeoRefWithSbetAlgorithm *QgsPointCloudGeoRefWithSbetAlgorithm::createInstance() const
{
  return new QgsPointCloudGeoRefWithSbetAlgorithm();
}

void QgsPointCloudGeoRefWithSbetAlgorithm::addAlgorithmParams()
{
  QStringList rasterDataTypes = QStringList();
  rasterDataTypes << QStringLiteral( "Byte" )
                  << QStringLiteral( "Integer16" )
                  << QStringLiteral( "Unsigned Integer16" )
                  << QStringLiteral( "Integer32" )
                  << QStringLiteral( "Unsigned Integer32" )
                  << QStringLiteral( "Float32" )
                  << QStringLiteral( "Float64" );

  std::unique_ptr< QgsProcessingParameterDefinition > rasterTypeParameter = qgis::make_unique< QgsProcessingParameterEnum >( QStringLiteral( "OUTPUT_TYPE" ), QObject::tr( "Output raster data type" ),  rasterDataTypes, false, 5, false );
  rasterTypeParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( rasterTypeParameter.release() );

  std::unique_ptr< QgsProcessingParameterNumber > lowerBoundParameter = qgis::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "LOWER_BOUND" ), QStringLiteral( "Lower bound for random number range" ), QgsProcessingParameterNumber::Double, QVariant(), true );
  lowerBoundParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( lowerBoundParameter.release() );

  std::unique_ptr< QgsProcessingParameterNumber > upperBoundParameter = qgis::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "UPPER_BOUND" ), QStringLiteral( "Upper bound for random number range" ), QgsProcessingParameterNumber::Double, QVariant(), true );
  upperBoundParameter->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( upperBoundParameter.release() );
}

Qgis::DataType QgsPointCloudGeoRefWithSbetAlgorithm::getPcdDataType( int typeId )
{
  switch ( typeId )
  {
    case 0:
      return Qgis::Byte;
    case 1:
      return Qgis::Int16;
    case 2:
      return Qgis::UInt16;
    case 3:
      return Qgis::Int32;
    case 4:
      return Qgis::UInt32;
    case 5:
      return Qgis::Float32;
    case 6:
      return Qgis::Float64;
    default:
      return Qgis::Float32;
  }
}

bool QgsPointCloudGeoRefWithSbetAlgorithm::getPcdInfo( const QVariantMap &parameters, QgsProcessingContext &context )
{
  mRandomUpperBound = parameterAsDouble( parameters, QStringLiteral( "UPPER_BOUND" ), context );
  mRandomLowerBound = parameterAsDouble( parameters, QStringLiteral( "LOWER_BOUND" ), context );

  if ( mRandomLowerBound > mRandomUpperBound )
    throw QgsProcessingException( QObject::tr( "The chosen lower bound for random number range is greater than the upper bound. The lower bound value must be smaller than the upper bound value." ) );

  int typeId = parameterAsInt( parameters, QStringLiteral( "OUTPUT_TYPE" ), context );
  Qgis::DataType rasterDataType = getPcdDataType( typeId );

  switch ( rasterDataType )
  {
    case Qgis::Byte:
      if ( mRandomLowerBound < std::numeric_limits<quint8>::min() || mRandomUpperBound > std::numeric_limits<quint8>::max() )
        throw QgsProcessingException( QObject::tr( "Raster datasets of type %3 only accept positive values between %1 and %2. Please choose other bounds for random values." ).arg( std::numeric_limits<quint8>::min() ).arg( std::numeric_limits<quint8>::max() ).arg( QLatin1String( "Byte" ) ) );
      if ( ( qgsDoubleNear( mRandomLowerBound, 0.0 ) && qgsDoubleNear( mRandomUpperBound, 0.0 ) ) || qgsDoubleNear( mRandomUpperBound, mRandomLowerBound ) )
      {
        //if parameters unset (=both are 0 or equal) --> use the whole value range
        mRandomUpperBound = std::numeric_limits<quint8>::max();
        mRandomLowerBound = std::numeric_limits<quint8>::min();
      }
      break;
    case Qgis::Int16:
      if ( mRandomLowerBound < std::numeric_limits<qint16>::min() || mRandomUpperBound > std::numeric_limits<qint16>::max() )
        throw QgsProcessingException( QObject::tr( "Raster datasets of type %3 only accept values between %1 and %2. Please choose other bounds for random values." ).arg( std::numeric_limits<qint16>::min() ).arg( std::numeric_limits<qint16>::max() ).arg( QLatin1String( "Integer16" ) ) );
      if ( ( qgsDoubleNear( mRandomLowerBound, 0.0 ) && qgsDoubleNear( mRandomUpperBound, 0.0 ) ) || qgsDoubleNear( mRandomUpperBound, mRandomLowerBound ) )
      {
        mRandomUpperBound = std::numeric_limits<qint16>::max();
        mRandomLowerBound = std::numeric_limits<qint16>::min();
      }
      break;
    case Qgis::UInt16:
      if ( mRandomLowerBound < std::numeric_limits<quint16>::min() || mRandomUpperBound > std::numeric_limits<quint16>::max() )
        throw QgsProcessingException( QObject::tr( "Raster datasets of type %3 only accept positive values between %1 and %2. Please choose other bounds for random values." ).arg( std::numeric_limits<quint16>::min() ).arg( std::numeric_limits<quint16>::max() ).arg( QLatin1String( "Unsigned Integer16" ) ) );
      if ( ( qgsDoubleNear( mRandomLowerBound, 0.0 ) && qgsDoubleNear( mRandomUpperBound, 0.0 ) ) || qgsDoubleNear( mRandomUpperBound, mRandomLowerBound ) )
      {
        mRandomUpperBound = std::numeric_limits<quint16>::max();
        mRandomLowerBound = std::numeric_limits<quint16>::min();
      }
      break;
    case Qgis::Int32:
      if ( mRandomLowerBound < std::numeric_limits<qint32>::min() || mRandomUpperBound > std::numeric_limits<qint32>::max() )
        throw QgsProcessingException( QObject::tr( "Raster datasets of type %3 only accept values between %1 and %2. Please choose other bounds for random values." ).arg( std::numeric_limits<qint32>::min() ).arg( std::numeric_limits<qint32>::max() ).arg( QLatin1String( "Integer32" ) ) );
      if ( ( qgsDoubleNear( mRandomLowerBound, 0.0 ) && qgsDoubleNear( mRandomUpperBound, 0.0 ) ) || qgsDoubleNear( mRandomUpperBound, mRandomLowerBound ) )
      {
        mRandomUpperBound = std::numeric_limits<qint32>::max();
        mRandomLowerBound = std::numeric_limits<qint32>::min();
      }
      break;
    case Qgis::UInt32:
      if ( mRandomLowerBound < std::numeric_limits<quint32>::min() || mRandomUpperBound > std::numeric_limits<quint32>::max() )
        throw QgsProcessingException( QObject::tr( "Raster datasets of type %3 only accept positive values between %1 and %2. Please choose other bounds for random values." ).arg( std::numeric_limits<quint32>::min() ).arg( std::numeric_limits<quint32>::max() ).arg( QLatin1String( "Unsigned Integer32" ) ) );
      if ( ( qgsDoubleNear( mRandomLowerBound, 0.0 ) && qgsDoubleNear( mRandomUpperBound, 0.0 ) ) || qgsDoubleNear( mRandomUpperBound, mRandomLowerBound ) )
      {
        mRandomUpperBound = std::numeric_limits<quint32>::max();
        mRandomLowerBound = std::numeric_limits<quint32>::min();
      }
      break;
    case Qgis::Float32:
      if ( ( qgsDoubleNear( mRandomLowerBound, 0.0 ) && qgsDoubleNear( mRandomUpperBound, 0.0 ) ) || qgsDoubleNear( mRandomUpperBound, mRandomLowerBound ) )
      {
        mRandomUpperBound = std::numeric_limits<float>::max();
        mRandomLowerBound = std::numeric_limits<float>::min();
      }
      break;
    case Qgis::Float64:
      if ( ( qgsDoubleNear( mRandomLowerBound, 0.0 ) && qgsDoubleNear( mRandomUpperBound, 0.0 ) ) || qgsDoubleNear( mRandomUpperBound, mRandomLowerBound ) )
      {
        mRandomUpperBound = std::numeric_limits<double>::max();
        mRandomLowerBound = std::numeric_limits<double>::min();
      }
      break;
    default:
      break;
  }

  mRandomUniformIntDistribution = std::uniform_int_distribution<long>( mRandomLowerBound, mRandomUpperBound );
  mRandomUniformDoubleDistribution = std::uniform_real_distribution<double>( mRandomLowerBound, mRandomUpperBound );

  return true;
}

long QgsPointCloudGeoRefWithSbetAlgorithm::generateRandomLongValue( std::mt19937 &mersenneTwister )
{
  return mRandomUniformIntDistribution( mersenneTwister );
}

double QgsPointCloudGeoRefWithSbetAlgorithm::generateRandomDoubleValue( std::mt19937 &mersenneTwister )
{
  return mRandomUniformDoubleDistribution( mersenneTwister );
}

///@endcond
