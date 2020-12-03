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
// QgsPointCloudAlgorithmBase
//
QString QgsPointCloudAlgorithmBase::group() const
{
  return QObject::tr( "Point Cloud" );
}

QString QgsPointCloudAlgorithmBase::groupId() const
{
  return QStringLiteral( "pointcloud" );
}

void QgsPointCloudAlgorithmBase::initAlgorithm( const QVariantMap & va )
{
  //add specific parameters
  addAlgorithmParams();

}

bool QgsPointCloudAlgorithmBase::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );
  
  return true;
}
QVariantMap QgsPointCloudAlgorithmBase::processPointCloudAlgorithm(const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback)
{
  return QVariantMap();
}
QVariantMap QgsPointCloudAlgorithmBase::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  return processPointCloudAlgorithm(parameters,context,feedback);
}

//
//---------------------------------------------------------------QgsPointCloudGeoRefWithSbetAlgorithm----------------------------------------------------------------------------------------------------------
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
  addParameter(new QgsProcessingParameterFile(QStringLiteral("INPUT"), QStringLiteral("INPUT"), QgsProcessingParameterFile::Behavior::File, QObject::tr("")));
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
  QVariantMap outputs;
  std::string output; // 程序运行的输出
  
  runTranslate(inputpointcloud.toStdString()+ " "+ outputcloud.toStdString(),output);

  outputs.insert(QStringLiteral("OUTPUT"), QString::fromStdString(output));

  return outputs;
}

Qgis::DataType QgsPointCloudGeoRefWithSbetAlgorithm::getPcdDataType(int typeID)
{
  return Qgis::DataType::UInt16;
}



/*####################################################################################################################################################
//点云配准程序
  Input1: target  点云
  input2：move 点云 
  output： todo，实现点云写出
*/

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
  addParameter(new QgsProcessingParameterFile(QStringLiteral("INPUT"), QStringLiteral("INPUT"), QgsProcessingParameterFile::Behavior::File, QObject::tr("")));
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

/*
//
//点云的航带拼接功能
//  点云航带间平差：  多航带点云数据平差与拼接

*/

QString QgsPointCloudMergeAlgorithm::name() const
{
  return QStringLiteral("QgsPointCloudMergeAlgorithm");
}

QString QgsPointCloudMergeAlgorithm::displayName() const
{
  return QObject::tr("Merge Point Cloud Flight Strip)");
}

QStringList QgsPointCloudMergeAlgorithm::tags() const
{
  return QObject::tr("point cloud,geospatial,Merge").split(',');
}

QString QgsPointCloudMergeAlgorithm::shortHelpString() const
{
  return QObject::tr("点云航带间拼接");
}

QgsPointCloudMergeAlgorithm *QgsPointCloudMergeAlgorithm::createInstance() const
{
  return new QgsPointCloudMergeAlgorithm();
}

void QgsPointCloudMergeAlgorithm::addAlgorithmParams()
{
  addParameter(new QgsProcessingParameterFile(QStringLiteral("INPUT"), QStringLiteral("Point Cloud Folder"),QgsProcessingParameterFile::Behavior::Folder, QObject::tr("")));
}


bool QgsPointCloudMergeAlgorithm::getPcdInfo(const QVariantMap &parameters, QgsProcessingContext &context)
{


  return true;
}


QVariantMap QgsPointCloudMergeAlgorithm::processPointCloudAlgorithm(const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback)
{
  inputpointcloud = parameterAsFileList(parameters, QStringLiteral("INPUT"), context);
  
  outputcloud = parameterAsFileOutput(parameters, QStringLiteral("OUTPUT"), context);

  /*
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
  */
  QVariantMap outputs;
  outputs.insert(QStringLiteral("OUTPUT"), outputcloud);

  return outputs;
}

Qgis::DataType QgsPointCloudMergeAlgorithm::getPcdDataType(int typeID)
{
  return Qgis::DataType::UInt16;
}


/*
//
//点云地形滤波程序： 提取激光雷达地面点 --------------------------------------------------------------------------

*/

QString QgsPointCloudGroundFilterAlgorithm::name() const
{
  return QStringLiteral("QgsPointCloudGroundFilterAlgorithm");
}

QString QgsPointCloudGroundFilterAlgorithm::displayName() const
{
  return QObject::tr("地形滤波");
}

QStringList QgsPointCloudGroundFilterAlgorithm::tags() const
{
  return QObject::tr("point cloud,geospatial,Ground Filter").split(',');
}

QString QgsPointCloudGroundFilterAlgorithm::shortHelpString() const
{
  return QObject::tr("点云航带间拼接");
}

QgsPointCloudGroundFilterAlgorithm *QgsPointCloudGroundFilterAlgorithm::createInstance() const
{
  return new QgsPointCloudGroundFilterAlgorithm();
}

void QgsPointCloudGroundFilterAlgorithm::addAlgorithmParams()
{
  addParameter(new QgsProcessingParameterFile(QStringLiteral("INPUT"), QStringLiteral("Input Point Cloud"), QgsProcessingParameterFile::Behavior::File, QObject::tr("")));
  addParameter(new QgsProcessingParameterFileDestination(QStringLiteral("OUTPUT"), QObject::tr("Output point cloud")));
}


bool QgsPointCloudGroundFilterAlgorithm::getPcdInfo(const QVariantMap &parameters, QgsProcessingContext &context)
{


  return true;
}


QVariantMap QgsPointCloudGroundFilterAlgorithm::processPointCloudAlgorithm(const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback)
{
  inputpointcloud = parameterAsFileList(parameters, QStringLiteral("INPUT"), context);

  outputcloud = parameterAsFileOutput(parameters, QStringLiteral("OUTPUT"), context);

  /*
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
  */
  QVariantMap outputs;
  outputs.insert(QStringLiteral("OUTPUT"), outputcloud);

  return outputs;
}

Qgis::DataType QgsPointCloudGroundFilterAlgorithm::getPcdDataType(int typeID)
{
  return Qgis::DataType::UInt16;
}


/*
//
//点云与彩色影像配准功能：将点云图像变为彩色点云   ######################################################################################################
//
*/

QString QgsPointCloudGetColorAlgorithm::name() const
{
  return QStringLiteral("QgsPointCloudGetColorAlgorithm");
}

QString QgsPointCloudGetColorAlgorithm::displayName() const
{
  return QObject::tr("点云与彩色DOM融合");
}

QStringList QgsPointCloudGetColorAlgorithm::tags() const
{
  return QObject::tr("point cloud,Color").split(',');
}

QString QgsPointCloudGetColorAlgorithm::shortHelpString() const
{
  return QObject::tr("点云与彩色DOM融合");
}

QgsPointCloudGetColorAlgorithm *QgsPointCloudGetColorAlgorithm::createInstance() const
{
  return new QgsPointCloudGetColorAlgorithm();
}

void QgsPointCloudGetColorAlgorithm::addAlgorithmParams()
{
  addParameter(new QgsProcessingParameterFile(QStringLiteral("INPUT"), QStringLiteral("Input Point Cloud"), QgsProcessingParameterFile::Behavior::File, QObject::tr("")));
  addParameter(new QgsProcessingParameterFile(QStringLiteral("INPUT2"), QStringLiteral("Input Point Cloud"), QgsProcessingParameterFile::Behavior::File, QObject::tr("")));
  addParameter(new QgsProcessingParameterFileDestination(QStringLiteral("OUTPUT"), QObject::tr("Output point cloud")));
}


bool QgsPointCloudGetColorAlgorithm::getPcdInfo(const QVariantMap &parameters, QgsProcessingContext &context)
{


  return true;
}


QVariantMap QgsPointCloudGetColorAlgorithm::processPointCloudAlgorithm(const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback)
{
  inputpointcloud = parameterAsFileList(parameters, QStringLiteral("INPUT"), context);
  inputDOM = parameterAsFileList(parameters, QStringLiteral("INPUT2"), context);
  outputcloud = parameterAsFileOutput(parameters, QStringLiteral("OUTPUT"), context);

  /*
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
  */
  QVariantMap outputs;
  outputs.insert(QStringLiteral("OUTPUT"), outputcloud);

  return outputs;
}

Qgis::DataType QgsPointCloudGetColorAlgorithm::getPcdDataType(int typeID)
{
  return Qgis::DataType::UInt16;
}
///@endcond