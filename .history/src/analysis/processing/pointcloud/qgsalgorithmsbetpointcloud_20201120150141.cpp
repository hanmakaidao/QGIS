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
  return QObject::tr( "Raster creation" );
}

QString QgsPointCloudGeoReferenceAlgorithmBase::groupId() const
{
  return QStringLiteral( "rastercreation" );
}

void QgsPointCloudGeoReferenceAlgorithmBase::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterExtent( QStringLiteral( "EXTENT" ), QObject::tr( "Desired extent" ) ) );
  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "TARGET_CRS" ), QObject::tr( "Target CRS" ), QStringLiteral( "ProjectCrs" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "PIXEL_SIZE" ), QObject::tr( "Pixel size" ),
                QgsProcessingParameterNumber::Double, 1, false, 0.01 ) );

  //add specific parameters
  addAlgorithmParams();

  addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Output raster" ) ) );
}

bool QgsPointCloudGeoReferenceAlgorithmBase::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );
  mCrs = parameterAsCrs( parameters, QStringLiteral( "TARGET_CRS" ), context );
  mExtent = parameterAsExtent( parameters, QStringLiteral( "EXTENT" ), context, mCrs );
  mPixelSize = parameterAsDouble( parameters, QStringLiteral( "PIXEL_SIZE" ), context );

  return true;
}

QVariantMap QgsPointCloudGeoReferenceAlgorithmBase::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  int typeId = parameterAsInt( parameters, QStringLiteral( "OUTPUT_TYPE" ), context );
  //prepare specific parameters
  mRasterDataType = getPcdDataType( typeId );
  //getPcdInfo( parameters, context );

  std::random_device rd {};
  std::mt19937 mersenneTwister{rd()};

  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  QFileInfo fi( outputFile );
  const QString outputFormat = QgsRasterFileWriter::driverForExtension( fi.suffix() );

  int rows = std::max( std::ceil( mExtent.height() / mPixelSize ), 1.0 );
  int cols = std::max( std::ceil( mExtent.width() / mPixelSize ), 1.0 );

  //build new raster extent based on number of columns and cellsize
  //this prevents output cellsize being calculated too small
  QgsRectangle rasterExtent = QgsRectangle( mExtent.xMinimum(), mExtent.yMaximum() - ( rows * mPixelSize ), mExtent.xMinimum() + ( cols * mPixelSize ), mExtent.yMaximum() );

  std::unique_ptr< QgsRasterFileWriter > writer = qgis::make_unique< QgsRasterFileWriter >( outputFile );
  writer->setOutputProviderKey( QStringLiteral( "gdal" ) );
  writer->setOutputFormat( outputFormat );
  std::unique_ptr<QgsRasterDataProvider > provider( writer->createOneBandRaster( mRasterDataType, cols, rows, rasterExtent, mCrs ) );
  if ( !provider )
    throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( outputFile ) );
  if ( !provider->isValid() )
    throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( outputFile, provider->error().message( QgsErrorMessage::Text ) ) );

  double step = rows > 0 ? 100.0 / rows : 1;

  for ( int row = 0; row < rows ; row++ )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }
    //prepare raw data depending on raster data type
    QgsRasterBlock block( mRasterDataType, cols, 1 );
    switch ( mRasterDataType )
    {
      case Qgis::Byte:
      {
        std::vector<quint8> byteRow( cols );
        for ( int col = 0; col < cols; col++ )
        {
          byteRow[col] = static_cast<quint8>( generateRandomLongValue( mersenneTwister ) );
        }
        block.setData( QByteArray( reinterpret_cast<const char *>( byteRow.data() ), QgsRasterBlock::typeSize( Qgis::Byte ) * cols ) );
        break;
      }
      case Qgis::Int16:
      {
        std::vector<qint16> int16Row( cols );
        for ( int col = 0; col < cols; col++ )
        {
          int16Row[col] = static_cast<qint16>( generateRandomLongValue( mersenneTwister ) );
        }
        block.setData( QByteArray( reinterpret_cast<const char *>( int16Row.data() ), QgsRasterBlock::typeSize( Qgis::Int16 ) * cols ) );
        break;
      }
      case Qgis::UInt16:
      {
        std::vector<quint16> uInt16Row( cols );
        for ( int col = 0; col < cols; col++ )
        {
          uInt16Row[col] = static_cast<quint16>( generateRandomLongValue( mersenneTwister ) );
        }
        block.setData( QByteArray( reinterpret_cast<const char *>( uInt16Row.data() ), QgsRasterBlock::typeSize( Qgis::UInt16 ) * cols ) );
        break;
      }
      case Qgis::Int32:
      {
        std::vector<qint32> int32Row( cols );
        for ( int col = 0; col < cols; col++ )
        {
          int32Row[col] = generateRandomLongValue( mersenneTwister );
        }
        block.setData( QByteArray( reinterpret_cast<const char *>( int32Row.data() ), QgsRasterBlock::typeSize( Qgis::Int32 ) * cols ) );
        break;
      }
      case Qgis::UInt32:
      {
        std::vector<quint32> uInt32Row( cols );
        for ( int col = 0; col < cols; col++ )
        {
          uInt32Row[col] = static_cast<quint32>( generateRandomLongValue( mersenneTwister ) );
        }
        block.setData( QByteArray( reinterpret_cast<const char *>( uInt32Row.data() ), QgsRasterBlock::typeSize( Qgis::UInt32 ) * cols ) );
        break;
      }
      case Qgis::Float32:
      {
        std::vector<float> float32Row( cols );
        for ( int col = 0; col < cols; col++ )
        {
          float32Row[col] = static_cast<float>( generateRandomDoubleValue( mersenneTwister ) );
        }
        block.setData( QByteArray( reinterpret_cast<const char *>( float32Row.data() ), QgsRasterBlock::typeSize( Qgis::Float32 ) * cols ) );
        break;
      }
      case Qgis::Float64:
      {
        std::vector<double> float64Row( cols );
        for ( int col = 0; col < cols; col++ )
        {
          float64Row[col] = generateRandomDoubleValue( mersenneTwister );
        }
        block.setData( QByteArray( reinterpret_cast<const char *>( float64Row.data() ), QgsRasterBlock::typeSize( Qgis::Float64 ) * cols ) );
        break;
      }
      default:
        break;
    }
    provider->writeBlock( &block, 1, 0, row );
    feedback->setProgress( row * step );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), outputFile );
  return outputs;
}

//
//QgsPointCloudGeoRefWithSbetAlgorithm
//
QString QgsPointCloudGeoRefWithSbetAlgorithm::name() const
{
  return QStringLiteral( "createrandomuniformrasterlayer" );
}

QString QgsPointCloudGeoRefWithSbetAlgorithm::displayName() const
{
  return QObject::tr( "Create random raster layer (uniform distribution)" );
}

QStringList QgsPointCloudGeoRefWithSbetAlgorithm::tags() const
{
  return QObject::tr( "raster,create,random" ).split( ',' );
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
