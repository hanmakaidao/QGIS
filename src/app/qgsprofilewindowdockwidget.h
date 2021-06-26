/***************************************************************************
    qgsmapcanvasdockwidget.h
    ------------------------
    begin                : February 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 **************************************************************************/
#ifndef QGSPROFILEWINDOWDOCKWIDGET_H
#define QGSPROFILEWINDOWDOCKWIDGET_H

#include "ui_qgsprofilewindowdockwidgetbase.h"
#include "ui_pointcloudclassselection.h"
#include "ui_pointcloudtargetclassselection.h"
#include "ui_qgsclasssettingwindowdockwidgetbase.h"
#include "ui_qgsDLwindowdockwidgetbase.h"
#include "ui_3DpointsPickedDlg.h"
#include "qgsdockwidget.h"
#include "qgspointxy.h"
#include "qgis_app.h"
#include <QWidgetAction>
#include <QTimer>
#include <memory>

#include "View3D.h"
#include <QVector3D>

class QgsScaleComboBox;
class QgsDoubleSpinBox;
class QCheckBox;
class QRadioButton;

typedef View3D QgsProfileWinow;
typedef Imath::V3d Point3D;

class APP_EXPORT QgsClassSettingWindowDockWidget : public QgsDockWidget, private Ui::QgsClassSettingWindowDockWidgetBase
{
  Q_OBJECT
public:
    explicit QgsClassSettingWindowDockWidget(const QString &name, QWidget *parent = nullptr);
    std::map<int, bool> getoriginalClass()
    {
      return originalClass;
    }
    QString getTargetClass()
    {
      return TargetClass;
    }
private slots:
    void OnCheckChanged();
    void OnCheckChanged2();
    void CheckAll();
    void UnCheckAll();
    void myShowDock();
    void myHideDock();
private:
    bool initialized;
    QWidget* profile_widget;
    std::map<int, bool> originalClass;
    QString TargetClass;
};

class APP_EXPORT QgsProfileWindowDockWidget : public QgsDockWidget, private Ui::QgsProfileWindowDockWidgetBase
{
    Q_OBJECT
      friend QgsClassSettingWindowDockWidget;
  public:
    explicit QgsProfileWindowDockWidget( const QString &name, QWidget *parent = nullptr );

	QgsProfileWinow *getmapCanvas();
	void setProfileWindow(QgsProfileWinow * window);
	void setMain3DWindow(QgsProfileWinow * window);
  void setclassdock(QgsClassSettingWindowDockWidget* dock);

  private slots:
	void OnmActionSaveEditsClicked();
	void OnmselectiononprofileClciekd();

	void OndrawlieonprofileClicked2();

	void OnmActionPickPoints();

	void OnmActionBrushPoints();


	void ApplyButtonClicked();
	void OndrawlieonprofileClicked();
	void OnmActionToggleEditingClicked();
	void OnmActionsetshaderClicked();
	void OnmActionHandClicked();
  void rotatePointCloudLeft();
  void rotatePointCloudRight();

private:
    QgsProfileWinow *mMapCanvas = nullptr;
    QgsProfileWinow *mMainCanvas = nullptr;
    QgsClassSettingWindowDockWidget* classdock = nullptr;

    QRadioButton *mSyncExtentRadio = nullptr;
    QRadioButton *mSyncSelectionRadio = nullptr;
    QgsScaleComboBox *mScaleCombo = nullptr;
    QgsDoubleSpinBox *mMagnificationEdit = nullptr;
    QgsDoubleSpinBox *mScaleFactorWidget = nullptr;
    QCheckBox *mSyncScaleCheckBox = nullptr;
    QWidget* profile_widget = nullptr;
	  bool Editing = false;
	  QString m_rule;
	  QString m_method;
};

#endif // QGSPROFILEWINDOWDOCKWIDGET_H

