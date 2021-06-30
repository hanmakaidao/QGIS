/***************************************************************************
    qgspointcloudlayerstylewidget.cpp
    ---------------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudlayerstylewidget.h"
#include "qgspointcloudrendererpropertieswidget.h"
#include "qgsstyle.h"
#include "qgsapplication.h"
#include "qgsmaplayer.h"
#include "qgspointcloudlayer.h"

QgsPointCloudRendererWidgetFactory::QgsPointCloudRendererWidgetFactory( QObject *parent )
  : QObject( parent )
{
  setIcon( QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/symbology.svg" ) ) );
  setTitle( tr( "Symbology" ) );
}

QgsMapLayerConfigWidget *QgsPointCloudRendererWidgetFactory::createWidget( QgsMapLayer *layer, QgsMapCanvas *, bool, QWidget *parent ) const
{
  return new QgsPointCloudRendererPropertiesWidget( qobject_cast< QgsPointCloudLayer * >( layer ), QgsStyle::defaultStyle(), parent );
}

bool QgsPointCloudRendererWidgetFactory::supportLayerPropertiesDialog() const
{
  return true;
}

bool QgsPointCloudRendererWidgetFactory::supportsStyleDock() const
{
  return true;
}

bool QgsPointCloudRendererWidgetFactory::supportsLayer( QgsMapLayer *layer ) const
{
  return layer->type() == QgsMapLayerType::PointCloudLayer;
}

QString QgsPointCloudRendererWidgetFactory::layerPropertiesPagePositionHint() const
{
  return QStringLiteral( "mOptsPage_Metadata" );
}



QgsPointCloudLayer3DRendererWidgetFactory::QgsPointCloudLayer3DRendererWidgetFactory(QObject *parent) :
  QObject(parent)
{
  setIcon(QIcon(":/images/themes/default/3d.svg"));
  setTitle(tr("3D View"));
}

QgsMapLayerConfigWidget *QgsPointCloudLayer3DRendererWidgetFactory::createWidget(QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockWidget, QWidget *parent) const
{
  Q_UNUSED(dockWidget)
    /*
    QgsPointCloudLayer *pointCloudLayer = qobject_cast<QgsPointCloudLayer *>(layer);
  if (!pointCloudLayer)
    return nullptr;
  QgsPointCloudLayer3DRendererWidget *widget = new QgsPointCloudLayer3DRendererWidget(pointCloudLayer, canvas, parent);
  if (pointCloudLayer)
    widget->setRenderer(dynamic_cast<QgsPointCloudLayer3DRenderer *>(pointCloudLayer->renderer3D()));
  return widget;
  */
  return new QgsPointCloud3DRendererPropertiesWidget(qobject_cast<QgsPointCloudLayer *>(layer), QgsStyle::defaultStyle(), parent);
}

bool QgsPointCloudLayer3DRendererWidgetFactory::supportLayerPropertiesDialog() const
{
  return true;
}

bool QgsPointCloudLayer3DRendererWidgetFactory::supportsLayer(QgsMapLayer *layer) const
{
  return layer->type() == QgsMapLayerType::PointCloudLayer;
}

QString QgsPointCloudLayer3DRendererWidgetFactory::layerPropertiesPagePositionHint() const
{
  return QStringLiteral("mOptsPage_Metadata");
}

bool QgsPointCloudLayer3DRendererWidgetFactory::supportsStyleDock() const
{
  return true;
}
