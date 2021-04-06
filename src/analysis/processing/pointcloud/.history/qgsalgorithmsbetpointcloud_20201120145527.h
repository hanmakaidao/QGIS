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

#ifndef QGSRANDOMRASTERALGORITHM_H
#define QGSRANDOMRASTERALGORITHM_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsapplication.h"
#include "random"

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
    virtual Qgis::DataType getRasterDataType( int typeId ) = 0;
    virtual bool prepareRandomParameters( const QVariantMap &parameters, QgsProcessingContext &context ) = 0;

    /**
     * Processes a raster using the generateRandomIntValues method which is implemented in subclasses providing different fuzzy membership types.
     */
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) final;

    /**
     * Virtual methods for random number generation may be overridden by subclassed algorithms to use specific random number distributions.
     */
    virtual long generateRandomLongValue( std::mt19937 &mersenneTwister ) = 0;
    virtual double generateRandomDoubleValue( std::mt19937 &mersenneTwister ) = 0;

  private:
    QgsRectangle mExtent;
    QgsCoordinateReferenceSystem mCrs;
    double mPixelSize;
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

  protected:
    void addAlgorithmParams() final;
    Qgis::DataType getRasterDataType( int typeId ) final;
    bool prepareRandomParameters( const QVariantMap &parameters, QgsProcessingContext &context ) final;
    long generateRandomLongValue( std::mt19937 &mersenneTwister ) final;
    double generateRandomDoubleValue( std::mt19937 &mersenneTwister ) final;

  private:
    double mRandomUpperBound;
    double mRandomLowerBound;
    std::uniform_int_distribution<long> mRandomUniformIntDistribution;
    std::uniform_real_distribution<double> mRandomUniformDoubleDistribution;
};


///@endcond PRIVATE

#endif // QGSRANDOMRASTERALGORITHM_H
