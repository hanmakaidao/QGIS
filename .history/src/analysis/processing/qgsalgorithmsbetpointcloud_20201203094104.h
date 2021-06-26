/***************************************************************************
                         qgsalgorithmsbetpointcloud.h
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

#ifndef QGSPOINTCLOUDALGORITHM_H
#define QGSPOINTCLOUDALGORITHM_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsapplication.h"
#include "random"
#include <pdal/util/FileUtils.hpp>
#include <pdal/util/Utils.hpp>
#include <pdal/filters/TransformationFilter.hpp>
#include <pdal/io/BufferReader.hpp>
#include <pdal/io/LasReader.hpp>
#include <pdal/StageFactory.hpp>

#include <memory>
#include <iostream>
#include <sstream>
#include <string>

using namespace pdal;
using namespace Utils;
static int runTranslate(std::string const& cmdline, std::string& output)
{
  //QgsApplication::libexecPath();

  ///char *prefixPath = getenv("PDAL_PREFIX_PATH");
  const std::string cmd = "pdal translate";
  return run_shell_command(cmd + " " + cmdline, output);
}

///@cond PRIVATE

class QgsPointCloudGeoReferenceAlgorithmBase : public QgsProcessingAlgorithm
{
  public:
    QString group() const final;
    QString groupId() const final;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) final;

  protected:

    /**
     * Adds specific subclass algorithm parameters. The common parameters, such as raster destination, are automatically
     * added by the base class.
     */
    virtual void addAlgorithmParams() = 0;

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) final;

    /**
     * Prepares the random number algorithm subclass for execution.
     */
    virtual Qgis::DataType getPcdDataType( int typeId ) = 0;
    virtual bool getPcdInfo( const QVariantMap &parameters, QgsProcessingContext &context ) = 0;

    /**
     * Processes a raster using the generateRandomIntValues method which is implemented in subclasses providing different fuzzy membership types.
     */
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )final;


    virtual QVariantMap processPointCloudAlgorithm(const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback);



  private:
    //QgsRectangle mExtent;

    //Qgis::DataType mRasterDataType;
};


class QgsPointCloudGeoRefWithSbetAlgorithm : public QgsPointCloudGeoReferenceAlgorithmBase
{
  public:
    QgsPointCloudGeoRefWithSbetAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmRandomRaster.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmRandomRaster.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsPointCloudGeoRefWithSbetAlgorithm *createInstance() const override SIP_FACTORY;
    QVariantMap processPointCloudAlgorithm(const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback) final;

  protected:
    void addAlgorithmParams() final;
    Qgis::DataType getPcdDataType( int typeId ) final;
    bool getPcdInfo( const QVariantMap &parameters, QgsProcessingContext &context ) final;

  private:

    QgsCoordinateReferenceSystem mCrs;
    QString inputpointcloud;
    QString inputsbet;
    QString outputcloud;
};


class QgsPointCloudIcpFilterAlgorithm : public QgsPointCloudGeoReferenceAlgorithmBase
{
public:
  QgsPointCloudIcpFilterAlgorithm() = default;
  QIcon icon() const override { return QgsApplication::getThemeIcon(QStringLiteral("/algorithms/mAlgorithmRandomRaster.svg")); }
  QString svgIconPath() const override { return QgsApplication::iconPath(QStringLiteral("/algorithms/mAlgorithmRandomRaster.svg")); }
  QString name() const override;
  QString displayName() const override;
  QStringList tags() const override;
  QString shortHelpString() const override;
  QgsPointCloudIcpFilterAlgorithm *createInstance() const override SIP_FACTORY;
  QVariantMap processPointCloudAlgorithm(const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback) final;

protected:
  void addAlgorithmParams() final;
  Qgis::DataType getPcdDataType(int typeId) final;
  bool getPcdInfo(const QVariantMap &parameters, QgsProcessingContext &context) final;

private:
  QString inputpointcloud;
  QString inputpointcloud2;
  QString outputcloud;
};


class QgsPointCloudMergeAlgorithm : public QgsPointCloudGeoReferenceAlgorithmBase
{
public:
  QgsPointCloudMergeAlgorithm() = default;
  QIcon icon() const override { return QgsApplication::getThemeIcon(QStringLiteral("/algorithms/mAlgorithmRandomRaster.svg")); }
  QString svgIconPath() const override { return QgsApplication::iconPath(QStringLiteral("/algorithms/mAlgorithmRandomRaster.svg")); }
  QString name() const override;
  QString displayName() const override;
  QStringList tags() const override;
  QString shortHelpString() const override;
  QgsPointCloudMergeAlgorithm *createInstance() const override SIP_FACTORY;
  QVariantMap processPointCloudAlgorithm(const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback) final;

protected:
  void addAlgorithmParams() final;
  Qgis::DataType getPcdDataType(int typeId) final;
  bool getPcdInfo(const QVariantMap &parameters, QgsProcessingContext &context) final;

private:
  QStringList inputpointcloud;
  //QString inputpointcloud2;
  QString outputcloud;
};

/*
点云地形滤波程序： 提取激光雷达地面点
*/

class QgsPointCloudGroundFilterAlgorithm : public QgsPointCloudGeoReferenceAlgorithmBase
{
public:
  QgsPointCloudGroundFilterAlgorithm() = default;
  QIcon icon() const override { return QgsApplication::getThemeIcon(QStringLiteral("/algorithms/mAlgorithmRandomRaster.svg")); }
  QString svgIconPath() const override { return QgsApplication::iconPath(QStringLiteral("/algorithms/mAlgorithmRandomRaster.svg")); }
  QString name() const override;
  QString displayName() const override;
  QStringList tags() const override;
  QString shortHelpString() const override;
  QgsPointCloudGroundFilterAlgorithm *createInstance() const override SIP_FACTORY;
  QVariantMap processPointCloudAlgorithm(const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback) final;

protected:
  void addAlgorithmParams() final;
  Qgis::DataType getPcdDataType(int typeId) final;
  bool getPcdInfo(const QVariantMap &parameters, QgsProcessingContext &context) final;

private:
  QStringList inputpointcloud;
  //QString inputpointcloud2;
  QString outputcloud;
};

/*
点云与彩色： 提取激光雷达地面点
*/

class QgsPointCloudGroundFilterAlgorithm : public QgsPointCloudGeoReferenceAlgorithmBase
{
public:
  QgsPointCloudGroundFilterAlgorithm() = default;
  QIcon icon() const override { return QgsApplication::getThemeIcon(QStringLiteral("/algorithms/mAlgorithmRandomRaster.svg")); }
  QString svgIconPath() const override { return QgsApplication::iconPath(QStringLiteral("/algorithms/mAlgorithmRandomRaster.svg")); }
  QString name() const override;
  QString displayName() const override;
  QStringList tags() const override;
  QString shortHelpString() const override;
  QgsPointCloudGroundFilterAlgorithm *createInstance() const override SIP_FACTORY;
  QVariantMap processPointCloudAlgorithm(const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback) final;

protected:
  void addAlgorithmParams() final;
  Qgis::DataType getPcdDataType(int typeId) final;
  bool getPcdInfo(const QVariantMap &parameters, QgsProcessingContext &context) final;

private:
  QStringList inputpointcloud;
  //QString inputpointcloud2;
  QString outputcloud;
};



///@endcond PRIVATE

#endif // QGSPOINTCLOUDALGORITHM_H
