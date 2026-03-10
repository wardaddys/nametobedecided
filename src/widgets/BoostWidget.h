/**
 * @file BoostWidget.h
 * @brief Comprehensive Boost & Forced Induction Widget
 */

#ifndef BOOSTWIDGET_H
#define BOOSTWIDGET_H

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QTableWidget>
#include <QWidget>

#include "TuningWidgetBase.h"

class BoostWidget : public TuningWidgetBase {
  Q_OBJECT

public:
  explicit BoostWidget(QWidget *parent = nullptr);
  ~BoostWidget() override;

  void loadFromECU() override;

private slots:
  void onInductionTypeChanged(int index);
  void onNoviceModeToggled(bool enabled);
  void on2DViewClicked();
  void on3DViewClicked();

private:
  void setupUi();
  QWidget *createHeaderSection();
  QWidget *createBoostTargetSection();
  QWidget *createTableSection();
  QWidget *create3DViewSection();
  QSlider *createStyledSlider(int min, int max, int value);

  // Header controls
  QComboBox *m_inductionTypeCombo;
  QProgressBar *m_setupProgress;
  QLabel *m_liveTuningLabel;
  QCheckBox *m_liveTuningToggle;

  // Boost Target
  QCheckBox *m_noviceModeToggle;
  QPushButton *m_2dButton;
  QPushButton *m_3dButton;

  // Table and 3D
  QTableWidget *m_boostTable;
  QWidget *m_3dViewPlaceholder;
};

#endif // BOOSTWIDGET_H
