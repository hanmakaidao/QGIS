/***************************************************************************
    qgsmeasuretool.cpp  -  map tool for measuring distances and areas
    ---------------------
    begin                : April 2007
    copyright            : (C) 2007 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsdistancearea.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaptopixel.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgssnappingutils.h"
#include "qgstolerance.h"
#include "qgsexception.h"
#include "qgsmeasuredialog.h"
#include "qgsmeasuretool.h"
#include "qgsmessagelog.h"
#include "qgssettings.h"
#include "qgsproject.h"
#include "qgssnapindicator.h"
#include "qgsmapmouseevent.h"

#include <QMessageBox>

#include "qgisapp.h"

QgsMeasureTool::QgsMeasureTool( QgsMapCanvas *canvas, bool measureArea )
  : QgsMapTool( canvas )
  , mMeasureArea( measureArea )
  , mSnapIndicator( new QgsSnapIndicator( canvas ) )
{
  mRubberBand = new QgsRubberBand( canvas, mMeasureArea ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry );
  mRubberBandPoints = new QgsRubberBand( canvas, QgsWkbTypes::PointGeometry );

  // Append point we will move
  mPoints.append( QgsPointXY( 0, 0 ) );
  mDestinationCrs = canvas->mapSettings().destinationCrs();

  mDialog = new QgsMeasureDialog( this );
  mDialog->setWindowFlags( mDialog->windowFlags() | Qt::Tool );
  mDialog->restorePosition();

  connect( canvas, &QgsMapCanvas::destinationCrsChanged, this, &QgsMeasureTool::updateSettings );
}

QgsMeasureTool::~QgsMeasureTool()
{
  // important - dialog is not parented to this tool (it's parented to the main window)
  // but we want to clean it up now
  delete mDialog;
}

QVector<QgsPointXY> QgsMeasureTool::points() const
{
  return mPoints;
}


void QgsMeasureTool::activate()
{
  mDialog->show();
  mRubberBand->show();
  mRubberBandPoints->show();
  QgsMapTool::activate();

  // ensure that we have correct settings
  updateSettings();

  // If we suspect that they have data that is projected, yet the
  // map CRS is set to a geographic one, warn them.
  if ( mCanvas->mapSettings().destinationCrs().isValid() &&
       mCanvas->mapSettings().destinationCrs().isGeographic() &&
       ( mCanvas->extent().height() > 360 ||
         mCanvas->extent().width() > 720 ) )
  {
    QMessageBox::warning( nullptr, tr( "Incorrect Measure Results" ),
                          tr( "<p>This map is defined with a geographic coordinate system "
                              "(latitude/longitude) "
                              "but the map extents suggests that it is actually a projected "
                              "coordinate system (e.g., Mercator). "
                              "If so, the results from line or area measurements will be "
                              "incorrect.</p>"
                              "<p>To fix this, explicitly set an appropriate map coordinate "
                              "system using the <tt>Settings:Project Properties</tt> menu." ) );
    mWrongProjectProjection = true;
  }
}

void QgsMeasureTool::deactivate()
{
  mSnapIndicator->setMatch( QgsPointLocator::Match() );

  mDialog->hide();
  mRubberBand->hide();
  mRubberBandPoints->hide();
  QgsMapTool::deactivate();
}

void QgsMeasureTool::restart()
{
  mPoints.clear();

  mRubberBand->reset( mMeasureArea ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry );
  mRubberBandPoints->reset( QgsWkbTypes::PointGeometry );

  mDone = true;
  mWrongProjectProjection = false;
}

void QgsMeasureTool::updateSettings()
{
  QgsSettings settings;

  int myRed = settings.value( QStringLiteral( "qgis/default_measure_color_red" ), 222 ).toInt();
  int myGreen = settings.value( QStringLiteral( "qgis/default_measure_color_green" ), 155 ).toInt();
  int myBlue = settings.value( QStringLiteral( "qgis/default_measure_color_blue" ), 67 ).toInt();
  mRubberBand->setColor( QColor( myRed, myGreen, myBlue, 100 ) );
  mRubberBand->setWidth( 3 );
  mRubberBandPoints->setIcon( QgsRubberBand::ICON_CIRCLE );
  mRubberBandPoints->setIconSize( 10 );
  mRubberBandPoints->setColor( QColor( myRed, myGreen, myBlue, 150 ) );

  // Reproject the points to the new destination CoordinateReferenceSystem
  if ( mRubberBand->size() > 0 && mDestinationCrs != mCanvas->mapSettings().destinationCrs() && mCanvas->mapSettings().destinationCrs().isValid() )
  {
    QVector<QgsPointXY> points = mPoints;
    bool lastDone = mDone;

    mDialog->restart();
    mDone = lastDone;
    QgsCoordinateTransform ct( mDestinationCrs, mCanvas->mapSettings().destinationCrs(), QgsProject::instance() );

    const auto constPoints = points;
    for ( const QgsPointXY &previousPoint : constPoints )
    {
      try
      {
        QgsPointXY point = ct.transform( previousPoint );

        mPoints.append( point );
        mRubberBand->addPoint( point, false );
        mRubberBandPoints->addPoint( point, false );
      }
      catch ( QgsCsException &cse )
      {
        QgsMessageLog::logMessage( tr( "Transform error caught at the MeasureTool: %1" ).arg( cse.what() ) );
      }
    }

    mRubberBand->updatePosition();
    mRubberBand->update();
    mRubberBandPoints->updatePosition();
    mRubberBandPoints->update();
  }
  mDestinationCrs = mCanvas->mapSettings().destinationCrs();

  mDialog->updateSettings();

  if ( !mDone && mRubberBand->size() > 0 )
  {
    mRubberBand->addPoint( mPoints.last() );
    mDialog->addPoint();
  }
  if ( mRubberBand->size() > 0 )
  {
    mRubberBand->setVisible( true );
    mRubberBandPoints->setVisible( true );
  }
}

//////////////////////////

void QgsMeasureTool::canvasPressEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e )
}

void QgsMeasureTool::canvasMoveEvent( QgsMapMouseEvent *e )
{
  QgsPointXY point = e->snapPoint();
  mSnapIndicator->setMatch( e->mapPointMatch() );

  if ( ! mDone )
  {
    mRubberBand->movePoint( point );
    mDialog->mouseMove( point );
  }
}


void QgsMeasureTool::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  QgsPointXY point = e->snapPoint();

  if ( mDone ) // if we have stopped measuring any mouse click restart measuring
  {
    mDialog->restart();
  }

  if ( e->button() == Qt::RightButton ) // if we clicked the right button we stop measuring
  {
    mDone = true;
    mRubberBand->removeLastPoint();
    mDialog->removeLastPoint();
  }
  else if ( e->button() == Qt::LeftButton )
  {
    mDone = false;
    addPoint( point );
  }

  mDialog->show();

}

void QgsMeasureTool::undo()
{
  if ( mRubberBand )
  {
    if ( mPoints.empty() )
    {
      return;
    }

    if ( mPoints.size() == 1 )
    {
      //removing first point, so restart everything
      restart();
      mDialog->restart();
    }
    else
    {
      //remove second last point from line band, and last point from points band
      mRubberBand->removePoint( -2, true );
      mRubberBandPoints->removePoint( -1, true );
      mPoints.removeLast();

      mDialog->removeLastPoint();
    }

  }
}

void QgsMeasureTool::keyPressEvent( QKeyEvent *e )
{
  if ( ( e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete ) )
  {
    if ( !mDone )
    {
      undo();
    }

    // Override default shortcut management in MapCanvas
    e->ignore();
  }
}


void QgsMeasureTool::addPoint( const QgsPointXY &point )
{
  QgsDebugMsg( "point=" + point.toString() );

  // don't add points with the same coordinates
  if ( !mPoints.isEmpty() && mPoints.last() == point )
  {
    return;
  }

  QgsPointXY pnt( point );
  // Append point that we will be moving.
  mPoints.append( pnt );

  mRubberBand->addPoint( point );
  mRubberBandPoints->addPoint( point );
  if ( ! mDone )    // Prevent the insertion of a new item in segments measure table
  {
    mDialog->addPoint();
  }
}

//////--------------------------------------剖面图---------------------------------------------------------

QgsPointCloudProfileTool::QgsPointCloudProfileTool( QgsMapCanvas *canvas, bool measureArea, View3D* window_3d)
  : QgsMapTool( canvas )
  , mMeasureArea( measureArea )
  , mSnapIndicator( new QgsSnapIndicator( canvas ) )
{
  mRubberBand = new QgsRubberBand( canvas, mMeasureArea ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry );
  mRubberBandPoints = new QgsRubberBand( canvas, QgsWkbTypes::PointGeometry );

  // Append point we will move
  mPoints.append( QgsPointXY( 0, 0 ) );
  mDestinationCrs = canvas->mapSettings().destinationCrs();

  //mDialog = new QgsMeasureDialog( this );
 // mDialog->setWindowFlags( mDialog->windowFlags() | Qt::Tool );
  //mDialog->restorePosition();

  connect( canvas, &QgsMapCanvas::destinationCrsChanged, this, &QgsPointCloudProfileTool::updateSettings );

  ///
  window_3d = window_3d;
}


QgsPointCloudProfileTool::~QgsPointCloudProfileTool()
{
  // important - dialog is not parented to this tool (it's parented to the main window)
  // but we want to clean it up now
 // delete mDialog;
}

QVector<QgsPointXY> QgsPointCloudProfileTool::points() const
{
  return mPoints;
}


void QgsPointCloudProfileTool::activate()
{
 // mDialog->show();
  mRubberBand->show();
  mRubberBandPoints->show();
  QgsMapTool::activate();

  // ensure that we have correct settings
  updateSettings();

  // If we suspect that they have data that is projected, yet the
  // map CRS is set to a geographic one, warn them.
  if ( mCanvas->mapSettings().destinationCrs().isValid() &&
       mCanvas->mapSettings().destinationCrs().isGeographic() &&
       ( mCanvas->extent().height() > 360 ||
         mCanvas->extent().width() > 720 ) )
  {
  /*  QMessageBox::warning( nullptr, tr( "Incorrect Measure Results" ),
                          tr( "<p>This map is defined with a geographic coordinate system "
                              "(latitude/longitude) "
                              "but the map extents suggests that it is actually a projected "
                              "coordinate system (e.g., Mercator). "
                              "If so, the results from line or area measurements will be "
                              "incorrect.</p>"
                              "<p>To fix this, explicitly set an appropriate map coordinate "
                              "system using the <tt>Settings:Project Properties</tt> menu." ) );*/
    mWrongProjectProjection = true;
  }
}

void QgsPointCloudProfileTool::deactivate()
{
  mSnapIndicator->setMatch( QgsPointLocator::Match() );

  //mDialog->hide();
  mRubberBand->hide();
  mRubberBandPoints->hide();
  QgsMapTool::deactivate();
}

void QgsPointCloudProfileTool::restart()
{
  mPoints.clear();
  mRubberBand->reset(mMeasureArea ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry);
  //mRubberBand->reset( QgsWkbTypes::PolygonGeometry );
  mRubberBandPoints->reset( QgsWkbTypes::PointGeometry );

  mDone = true;
  mWrongProjectProjection = false;
}

void QgsPointCloudProfileTool::updateSettings()
{
  QgsSettings settings;

  int myRed = settings.value( QStringLiteral( "qgis/default_measure_color_red" ), 222 ).toInt();
  int myGreen = settings.value( QStringLiteral( "qgis/default_measure_color_green" ), 155 ).toInt();
  int myBlue = settings.value( QStringLiteral( "qgis/default_measure_color_blue" ), 67 ).toInt();
  mRubberBand->setColor( QColor( myRed, myGreen, myBlue, 100 ) );
  mRubberBand->setWidth( 3 );
  mRubberBandPoints->setIcon( QgsRubberBand::ICON_CIRCLE );
  mRubberBandPoints->setIconSize( 10 );
  mRubberBandPoints->setColor( QColor( myRed, myGreen, myBlue, 150 ) );

  // Reproject the points to the new destination CoordinateReferenceSystem
  if ( mRubberBand->size() > 0 && mDestinationCrs != mCanvas->mapSettings().destinationCrs() && mCanvas->mapSettings().destinationCrs().isValid() )
  {
    QVector<QgsPointXY> points = mPoints;
    bool lastDone = mDone;

   // mDialog->restart();
    mDone = lastDone;
    QgsCoordinateTransform ct( mDestinationCrs, mCanvas->mapSettings().destinationCrs(), QgsProject::instance() );

    const auto constPoints = points;
    for ( const QgsPointXY &previousPoint : constPoints )
    {
      try
      {
        QgsPointXY point = ct.transform( previousPoint );

        mPoints.append( point );
        mRubberBand->addPoint( point, false );
        mRubberBandPoints->addPoint( point, false );
      }
      catch ( QgsCsException &cse )
      {
        QgsMessageLog::logMessage( tr( "Transform error caught at the MeasureTool: %1" ).arg( cse.what() ) );
      }
    }

    mRubberBand->updatePosition();
    mRubberBand->update();
    mRubberBandPoints->updatePosition();
    mRubberBandPoints->update();
  }
  mDestinationCrs = mCanvas->mapSettings().destinationCrs();

  //mDialog->updateSettings();

  if ( !mDone && mRubberBand->size() > 0 )
  {
    mRubberBand->addPoint( mPoints.last() );
    //mDialog->addPoint();
  }
  if ( mRubberBand->size() > 0 )
  {
    mRubberBand->setVisible( true );
    mRubberBandPoints->setVisible( true );
  }
}

//////////////////////////

void QgsPointCloudProfileTool::canvasPressEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e )
}

void QgsPointCloudProfileTool::canvasMoveEvent( QgsMapMouseEvent *e )
{
  QgsPointXY point = e->snapPoint();
  mSnapIndicator->setMatch( e->mapPointMatch() );

  if ( ! mDone )
  {
    mRubberBand->movePoint( point );
   // mDialog->mouseMove( point );
  }
}


void QgsPointCloudProfileTool::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  QgsPointXY point = e->snapPoint();

  if ( mDone ) // if we have stopped measuring any mouse click restart measuring
  {
 //   mDialog->restart();
	  restart();
  }

  if ( e->button() == Qt::RightButton ) // if we clicked the right button we stop measuring
  {
	  if (mMeasureArea)
	  {
		  mDone = true;
		  mRubberBand->removeLastPoint();
		  m_V2FPoints.clear();
		  for (size_t i = 0; i < mPoints.size(); i++)
		  {
			  m_V2FPoints.push_back(Imath::V3f(mPoints[i].x(), mPoints[i].y(), 100.0));
		  }
		  QgisApp::instance()->On2dProfile();
	  }
	  if (mMeasureArea == false)
	  {
		  mDone = true;
		  mRubberBand->removeLastPoint();
		  m_V2FPoints.clear();

		  //@1 �����һ����͵ڶ����㹹�ɵ�ֱ�߷��̡�������		 
		  Imath::V2f startpoint(mPoints[0].x(), mPoints[0].y());
		  Imath::V2f secondpoint(mPoints[1].x(), mPoints[1].y());
		  Imath::V2f thirdpoint(mPoints[2].x(), mPoints[2].y());
		//@2 ����������������ֱ�ߵĴ��߾���
		  Imath::V2f b = secondpoint - startpoint;
		  Imath::V2f a = thirdpoint - startpoint;
		  float s = a.cross(b);
		  float d = Imath::abs(s/b.length());
		  Imath::V2f normline(mPoints[1].y()- mPoints[0].y(), mPoints[0].x()- mPoints[1].x()); // ֱ�ߵķ����� 
		  Imath::V2f normedline = normline.normalize();		  
		  //@3 �����ı��ε��ĸ���������� 
		  Imath::V2f pt1 = startpoint + normedline* d;
		  m_V2FPoints.push_back(Imath::V3f(pt1.x, pt1.y, 100.0));
		  Imath::V2f pt2 = startpoint - normedline* d;
		  m_V2FPoints.push_back(Imath::V3f(pt2.x, pt2.y, 100.0));
		  Imath::V2f pt3 = secondpoint - normedline* d;
		  m_V2FPoints.push_back(Imath::V3f(pt3.x, pt3.y, 100.0));
		  Imath::V2f pt4 = secondpoint + normedline* d;
		  m_V2FPoints.push_back(Imath::V3f(pt4.x, pt4.y, 100.0));

		  QgisApp::instance()->On2dProfile();
	  }

  }
  else if ( e->button() == Qt::LeftButton )
  {
	  if (mMeasureArea == false)
	  {
		  mDone = false;
		  if (mPoints.size()>2)
		  {
			  mRubberBand->removePoint(-2, true);
			  mRubberBandPoints->removePoint(-1, true);
			  mPoints.removeLast();
		  }
		  addPoint(point);
		  emit passPoint(point.x(), point.y());
	  }
	  if (mMeasureArea )
	  {
		  mDone = false;
		  addPoint(point);
		  emit passPoint(point.x(), point.y());
	  }
  }
}

void QgsPointCloudProfileTool::undo()
{
  if ( mRubberBand )
  {
    if ( mPoints.empty() )
    {
      return;
    }

    if ( mPoints.size() == 1 )
    {
      //removing first point, so restart everything
      restart();
     // mDialog->restart();
    }
    else
    {
      //remove second last point from line band, and last point from points band
      mRubberBand->removePoint( -2, true );
      mRubberBandPoints->removePoint( -1, true );
      mPoints.removeLast();
    }

  }
}

void QgsPointCloudProfileTool::keyPressEvent( QKeyEvent *e )
{
  if ( ( e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete ) )
  {
    if ( !mDone )
    {
      undo();
    }

    // Override default shortcut management in MapCanvas
    e->ignore();
  }
}


void QgsPointCloudProfileTool::addPoint( const QgsPointXY &point )
{
  QgsDebugMsg( "point=" + point.toString() );

  // don't add points with the same coordinates
  if ( !mPoints.isEmpty() && mPoints.last() == point )
  {
    return;
  }

  QgsPointXY pnt( point );
  // Append point that we will be moving.
  mPoints.append( pnt );

  mRubberBand->addPoint( point );
  mRubberBandPoints->addPoint( point );
}
