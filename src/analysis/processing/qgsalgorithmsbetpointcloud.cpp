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

void QgsPointCloudGeoReferenceAlgorithmBase::initAlgorithm( const QVariantMap & va )
{
 // const QString &name, const QString &description = QString(), Behavior behavior = File, const QString &extension = QString(), const QVariant &defaultValue = QVariant(),
 //                               bool optional = false, const QString &fileFilter = QString()
  addParameter( new QgsProcessingParameterFile( QStringLiteral( "INPUT" ),QStringLiteral( "INPUT" ), QgsProcessingParameterFile::Behavior::File,QObject::tr("")) );
  
  
  //addParameter( new QgsProcessingParameterNumber( QStringLiteral( "PIXEL_SIZE" ), QObject::tr( "Pixel size" ),
  //              QgsProcessingParameterNumber::Double, 1, false, 0.01 ) );

  //add specific parameters
  addAlgorithmParams();

  
}

bool QgsPointCloudGeoReferenceAlgorithmBase::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );
  
  return true;
}
QVariantMap QgsPointCloudGeoReferenceAlgorithmBase::processPointCloudAlgorithm(const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback)
{
  return QVariantMap();
}
QVariantMap QgsPointCloudGeoReferenceAlgorithmBase::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  return processPointCloudAlgorithm(parameters,context,feedback);
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
  addParameter(new QgsProcessingParameterFile(QStringLiteral("Sbet File"), QStringLiteral("Sbel file ,pos information"), QgsProcessingParameterFile::Behavior::File));
  addParameter(new QgsProcessingParameterCrs(QStringLiteral("TARGET_CRS"), QObject::tr("Target CRS"), QStringLiteral("ProjectCrs")));
  addParameter(new QgsProcessingParameterFileDestination(QStringLiteral("OUTPUT"), QObject::tr("Output point cloud")));
}

bool QgsPointCloudGeoRefWithSbetAlgorithm::getPcdInfo( const QVariantMap &parameters, QgsProcessingContext &context )
{
  return true;
}

QVariantMap QgsPointCloudGeoRefWithSbetAlgorithm::processPointCloudAlgorithm(const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback)
{

  mCrs = parameterAsCrs(parameters, QStringLiteral("TARGET_CRS"), context);
  inputpointcloud = parameterAsFile(parameters, QStringLiteral("INPUT"), context);
  inputsbet = parameterAsFile(parameters, QStringLiteral("Sbet File"), context);
  outputcloud = parameterAsFileOutput(parameters, QStringLiteral("OUTPUT"), context);


  std::string output;
  
  runTranslate(inputpointcloud.toStdString()+ " "+ outputcloud.toStdString(),output);
  
  QVariantMap outputs;
  outputs.insert(QStringLiteral("OUTPUT"), outputcloud);

  return outputs;
}

Qgis::DataType QgsPointCloudGeoRefWithSbetAlgorithm::getPcdDataType(int typeID)
{
  return Qgis::DataType::UInt16;
}



//
//QgsPointCloudGeoRefWithSbetAlgorithm
//
QString QgsPointCloudIcpFilterAlgorithm::name() const
{
  return QStringLiteral("QgsPointCloudIcpFilterAlgorithm");
}

QString QgsPointCloudIcpFilterAlgorithm::displayName() const
{
  return QObject::tr("ICP Point Cloud data file)");
}

QStringList QgsPointCloudIcpFilterAlgorithm::tags() const
{
  return QObject::tr("point cloud,geospatial,ICP").split(',');
}

QString QgsPointCloudIcpFilterAlgorithm::shortHelpString() const
{
  return QObject::tr("点云配准");
}

QgsPointCloudIcpFilterAlgorithm *QgsPointCloudIcpFilterAlgorithm::createInstance() const
{
  return new QgsPointCloudIcpFilterAlgorithm();
}

void QgsPointCloudIcpFilterAlgorithm::addAlgorithmParams()
{
  addParameter(new QgsProcessingParameterFile(QStringLiteral("INPUT2"), QStringLiteral("INPUT2"), QgsProcessingParameterFile::Behavior::File, QObject::tr("")));
}

bool QgsPointCloudIcpFilterAlgorithm::getPcdInfo(const QVariantMap &parameters, QgsProcessingContext &context)
{
  return true;
}


QVariantMap QgsPointCloudIcpFilterAlgorithm::processPointCloudAlgorithm(const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback)
{
  //mCrs = parameterAsCrs(parameters, QStringLiteral("TARGET_CRS"), context);
  inputpointcloud = parameterAsFile(parameters, QStringLiteral("INPUT"), context);

  inputpointcloud2 = parameterAsFile(parameters, QStringLiteral("INPUT2"), context);
  
  outputcloud = parameterAsFileOutput(parameters, QStringLiteral("OUTPUT"), context);


  Options options;
  options.add("filename", inputpointcloud.toStdString());
  std::unique_ptr<LasReader> reader1(new LasReader());
  reader1->setOptions(options);

  Options options2;
  options2.add("filename", inputpointcloud2.toStdString());
  std::unique_ptr<LasReader> reader2(new LasReader());
  reader2->setOptions(options2);

  pdal::StageFactory Factory;
  
  Stage *filter = Factory.createStage("filters.icp");


  filter->setInput(*reader1);
  filter->setInput(*reader2);

  PointTable table;
  filter->prepare(table);
  PointViewSet viewSet = filter->execute(table);

  MetadataNode root = filter->getMetadata();
  MetadataNode transform = root.findChild("transform");

  QVariantMap outputs;
  outputs.insert(QStringLiteral("OUTPUT"), outputcloud);

  return outputs;
}

Qgis::DataType QgsPointCloudIcpFilterAlgorithm::getPcdDataType(int typeID)
{
  return Qgis::DataType::UInt16;
}

///@endcond


///@endcond
