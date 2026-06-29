#ifndef ALLTABLESWIDGET_H
#define ALLTABLESWIDGET_H

#include "TableEditor.h"
#include "ColorLegendBar.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QSplitter>
#include <QTabWidget>
#include <QWidget>

#include "TunerMap3DWidget.h"
#include "core/ECUSettingsManager.h"

class SerialManager; // Forward declaration

class AllTablesWidget : public QWidget {
  Q_OBJECT

public:
  explicit AllTablesWidget(ECUSettingsManager *settingsManager, QWidget *parent = nullptr);
  ~AllTablesWidget();

public slots:
  void setVtecEnabled(bool enabled);
  void setLiveTuningEnabled(bool enabled);
  void setSerialManager(SerialManager *serial); // Added
  void onDefinitionsLoaded();

  // === Getters for Onboarding/ProductTour (Phase A) ===
  QWidget* getGraphContainer() const { return m_graphContainer; }
  QTabWidget* getMainViewTabs() const { return m_mainViewTabs; }
  QComboBox* getTableSelector() const { return m_tableSelector; }

private slots:
  void onTableSelectionChanged(int index);
  void onSearchTextChanged(const QString &text);
  void onTableDataReceived(const QString &tableName, const QVector<QVector<double>> &data);
  void onTableDataChanged();
  void onCopyTable();
  void onSmoothValues();
  void onInterpolate();

  void onAdjustSelected();
  void onCamProfileChanged(int index);
  void onMainTabChanged(int index);

private:
  void setupUi();
  void setupTableDropdown();
  void setup3DGraph();
  void populateTable(const QString &tableName);
  void update3DGraph();
  void updateVtecVisibility();

  // UI Components - Header
  QLabel *m_titleLabel;
  QLabel *m_subtitleLabel;

  // VTEC Warning Banner
  QFrame *m_vtecWarningBanner;
  bool m_vtecEnabled;
  bool m_liveTuningActive;

  // Table Selection
  QComboBox *m_tableSelector;
  QLineEdit *m_searchBox;

  // Table Info Header
  QLabel *m_tableInfoLabel;
  QLabel *m_tableDescLabel;

  // Main view tabs (Table | 3D Surface)
  QTabWidget *m_mainViewTabs;

  // Cam Profile Tabs (for VTEC)
  QTabWidget *m_camProfileTabs;
  TableEditor *m_lowCamEditor;
  TableEditor *m_highCamEditor;

  // Main Table Editor (non-VTEC mode)
  TableEditor *m_tableEditor;
  
  ColorLegendBar *m_colorLegendBar;

  // 3D Graph
  TunerMap3DWidget *m_graphContainer;

  // View Layout Management
  QWidget *m_tableTab;
  QWidget *m_dummyTable;
  QWidget *m_dummy3D;
  QWidget *m_dummySplit;
  QSplitter *m_splitter;

  // Right Panel Controls
  // Adjust Selected
  QDoubleSpinBox *m_adjustValue;
  QPushButton *m_adjustAddBtn;
  QPushButton *m_adjustSubBtn;

  // Quick Actions
  QPushButton *m_copyBtn;
  QPushButton *m_smoothBtn;
  QPushButton *m_interpolateBtn;

  // Current table data meta-info
  QString m_currentTableName;
  QStringList m_allTableNames; // T5: cached full list for search filtering
  bool m_gridMode;
  bool m_lightMode;
  
  ECUSettingsManager *m_settingsManager;
};

#endif // ALLTABLESWIDGET_H
