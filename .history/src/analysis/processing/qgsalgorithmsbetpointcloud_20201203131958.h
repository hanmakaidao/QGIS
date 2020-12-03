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
#include <pdal/util/Extractor.hpp>
#include <pdal/util/Georeference.hpp>
#include <pdal/util/IStream.hpp>

#include <pdal/io/BufferReader.hpp>
#include <pdal/io/LasReader.hpp>
#include <pdal/StageFactory.hpp>
#include <pdal/util/Georeference.hpp>

#include <pdal/Reader.hpp>
#include <pdal/PointTable.hpp>
#include <pdal/PointView.hpp>


#include <memory>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace pdal;
using namespace Utils;
static int runTranslate(std::string const& cmdline, std::string& output)
{
  const std::string cmd = "pdal translate";
  return run_shell_command(cmd + " " + cmdline, output);
}


static int runInfo(std::string const& cmdline, std::string& output)
{
  const std::string cmd = "pdal info";
  return run_shell_command(cmd + " " + cmdline, output);
}

typedef struct
{
    char signature[4];
    char vendorId[64]; //探测器
    char softwareVersion[32];
    float formatVersion;
    uint16_t headerSize;
    uint16_t gpsWeek;
    double minTime; // seconds
    double maxTime; // seconds
    uint32_t numRecords;
    uint16_t numStrips;
    uint32_t stripPointers[256];
    double misalignmentAngles[3]; // radians
    double imuOffsets[3];         // radians
    double temperature;           // degrees
    double pressure;              // mbar
    char freeSpace[830];
} RieglHeader;


typedef struct
{
    double gpsTime;
    uint8_t returnCount;
    float range[4]; // metres
    uint16_t intensity[4];
    float scanAngle;  // radians
    float roll;       // radians
    float pitch;      // radians
    float heading;    // radians
    double latitude;  // radians
    double longitude; // radians
    float elevation;  // metres
} RieglPulse;

// Optech does it like R3(heading) * R1(-pitch) * R2(-roll)
pdal::georeference::RotationMatrix createRieglRotationMatrix(double roll, double pitch, double heading)
{
    return georeference::RotationMatrix(
        std::cos(roll) * std::cos(heading) +
            std::sin(pitch) * std::sin(roll) * std::sin(heading), // m00
        std::cos(pitch) * std::sin(heading),                      // m01
        std::cos(heading) * std::sin(roll) -
            std::cos(roll) * std::sin(pitch) * std::sin(heading), // m02
        std::cos(heading) * std::sin(pitch) * std::sin(roll) -
            std::cos(roll) * std::sin(heading), // m10
        std::cos(pitch) * std::cos(heading),    // m11
        -std::sin(roll) * std::sin(heading) -
            std::cos(roll) * std::cos(heading) * std::sin(pitch), // m12
        -std::cos(pitch) * std::sin(roll),                        // m20
        std::sin(pitch),                                          // m21
        std::cos(pitch) * std::cos(roll)                          // m22
        );
}
///@cond PRIVATE

class RieglReaderWithGeoCorrect: public Reader
{
public:
    std::string getName() const;

    static const size_t MaximumNumberOfReturns = 4;
    static const size_t NumBytesInRecord = 69;
    static const size_t BufferSize = 1000000;
    static const size_t MaxNumRecordsInBuffer = BufferSize / NumBytesInRecord;

    RieglReaderWithGeoCorrect();

    const RieglReaderWithGeoCorrect& getHeader() const;

private:
    typedef std::vector<char> buffer_t;
    typedef buffer_t::size_type buffer_size_t;

    virtual void initialize();
    virtual void addDimensions(PointLayoutPtr layout);
    virtual void ready(PointTableRef table);
    virtual point_count_t read(PointViewPtr view, point_count_t num);
    size_t fillBuffer();
    virtual void done(PointTableRef table);

    RieglHeader m_header;
    georeference::RotationMatrix m_boresightMatrix;
    std::unique_ptr<IStream> m_istream;
    buffer_t m_buffer;
    LeExtractor m_extractor;
    size_t m_recordIndex;
    size_t m_returnIndex;
    RieglPulse m_pulse;
};

/*
关于点云数据处理算法的基础类
*/
class QgsPointCloudAlgorithmBase : public QgsProcessingAlgorithm
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
    QgsRectangle mExtent;
};

/*
  点云，依据自轨曲线，对点云XYZ进行校正的算法

*/
class QgsPointCloudGeoRefWithSbetAlgorithm : public QgsPointCloudAlgorithmBase
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
    void MyAlgorithm();
    
  private:

    QgsCoordinateReferenceSystem mCrs;
    QString inputpointcloud;
    QString inputsbet;
    QString outputcloud;
};


class QgsPointCloudIcpFilterAlgorithm : public QgsPointCloudAlgorithmBase
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


class QgsPointCloudMergeAlgorithm : public QgsPointCloudAlgorithmBase
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

class QgsPointCloudGroundFilterAlgorithm : public QgsPointCloudAlgorithmBase
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
点云与彩色影像配准功能：将点云图像变为彩色点云
*/

class QgsPointCloudGetColorAlgorithm : public QgsPointCloudAlgorithmBase
{
public:
  QgsPointCloudGetColorAlgorithm() = default;
  QIcon icon() const override { return QgsApplication::getThemeIcon(QStringLiteral("/algorithms/mAlgorithmRandomRaster.svg")); }
  QString svgIconPath() const override { return QgsApplication::iconPath(QStringLiteral("/algorithms/mAlgorithmRandomRaster.svg")); }
  QString name() const override;
  QString displayName() const override;
  QStringList tags() const override;
  QString shortHelpString() const override;
  QgsPointCloudGetColorAlgorithm *createInstance() const override SIP_FACTORY;
  QVariantMap processPointCloudAlgorithm(const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback) final;

protected:
  void addAlgorithmParams() final;
  Qgis::DataType getPcdDataType(int typeId) final;
  bool getPcdInfo(const QVariantMap &parameters, QgsProcessingContext &context) final;

private:
  QStringList inputpointcloud;
  QStringList inputDOM;
  QString outputcloud;
};



///@endcond PRIVATE

#endif // QGSPOINTCLOUDALGORITHM_H
