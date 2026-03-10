#ifndef ECUSETTINGSWIDGET_H
#define ECUSETTINGSWIDGET_H

#include <QGridLayout>
#include <QLineEdit>
#include <QTreeWidget>
#include <QStackedWidget>
#include <QWidget>
#include <QMap>
#include <QHash>
#include <QScrollArea>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLabel>

// Forward declarations for embedded widgets
class IdleControlWidget;
class ColdStartWidget;
class AccelEnrichmentWidget;
class TrimsWidget;
class VVTVTECWidget;
class BoostWidget;
class KnockWidget;
class LimitersWidget;
class IOConfigWidget;
class SerialManager;
class ECUSettingsManager;

#include "core/ECUData.h"

/**
 * @brief Comprehensive ECU Settings Widget
 * 
 * Implements all TunerStudio-style ECU settings including:
 * - Engine & Fuel Settings
 * - Limiters & Protection
 * - Auxiliary Controls
 * - Sensor Settings & Calibration
 * - IO Pins & Status
 */
class ECUSettingsWidget : public QWidget {
  Q_OBJECT

public:
  explicit ECUSettingsWidget(ECUSettingsManager *settingsManager, QWidget *parent = nullptr);

  // === Section 1.2: Public methods for MainWindow to wire serial connection ===
  void setSerialManager(SerialManager *serialManager);
  void readAllFromECU();
  void burnAllDirty();
  void updateRealtimeData(const RealTimeData &data);
  ECUSettingsManager* getSettingsManager() const { return m_settingsManager; }

private slots:
  void onSectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
  void onSearchTextChanged(const QString &text);
  void onSettingChanged(const QString &name, const QVariant &value);
  void onSettingsLoaded();

private:
  void setupUi();
  void populateSidebar();
  
  // Page creation methods
  QWidget* createEnginePage();
  QWidget* createSequentialFuelPage();
  QWidget* createGeneralSettingsPage();
  QWidget* createRevLimiterPage();
  QWidget* createShiftLightPage();
  QWidget* createEngineStatePage();
  QWidget* createTachoOutputPage();
  QWidget* createFanControlPage();
  QWidget* createTorqueConverterPage();
  QWidget* createAlternatorControlPage();
  QWidget* createBarometricPage();
  QWidget* createIATMAPPage();
  QWidget* createTPSLoadPage();
  QWidget* createMAPSettingsPage();
  QWidget* createMAFSettingsPage();
  QWidget* createMAFFlowCurvePage();
  QWidget* createMAFMATCorrectionPage();
  QWidget* createMATAirDensityPage();
  QWidget* createMATCLTCorrectionPage();
  QWidget* createRealtimeDisplayPage();
  QWidget* createIOPinsListPage();
  QWidget* createIOPinsUsagePage();
  QWidget* createCANbusPage();
  QWidget* createExpansionIOPage();
  QWidget* createGaugeLimitsPage();
  QWidget* createEGOPage();
  QWidget* createInjectorDeadTimePage();
  QWidget* createSensorCalibrationPage();
  QWidget* createCrankingAdvancePage();
  QWidget* createTechnicalReferencePage();
  QWidget* createInjectorDatabasePage();

  // Helper methods
  QWidget* createSettingsPage(const QString &title);
  void addComboField(QGridLayout *grid, int row, int col, 
                     const QString &label, const QStringList &options,
                     const QString &defaultVal = "");
  void addSpinField(QGridLayout *grid, int row, int col,
                    const QString &label, int min, int max, int defaultVal,
                    const QString &suffix = "");
  void addDoubleSpinField(QGridLayout *grid, int row, int col,
                          const QString &label, double min, double max, 
                          double defaultVal, const QString &suffix = "");
  void addCheckField(QGridLayout *grid, int row, int col,
                     const QString &label, bool checked = false);
  void addSensorCalibrationCombo(QGridLayout *grid, int row, int col,
                                  const QString &label, const QString &sensorType);

  // Bound control methods - these link UI controls to ECU settings
  QComboBox* addBoundCombo(QGridLayout *grid, int row, int col,
                           const QString &label, const QStringList &options,
                           const QString &settingName);
  QSpinBox* addBoundSpin(QGridLayout *grid, int row, int col,
                         const QString &label, const QString &settingName,
                         const QString &suffix = "");
  QDoubleSpinBox* addBoundDoubleSpin(QGridLayout *grid, int row, int col,
                                      const QString &label, const QString &settingName,
                                      const QString &suffix = "");
  QCheckBox* addBoundCheck(QGridLayout *grid, int row, int col,
                           const QString &label, const QString &settingName);
  
  void updateControlFromSetting(const QString &settingName);

  // UI Components
  QTreeWidget *m_sidebar;
  QStackedWidget *m_contentStack;
  QLineEdit *m_searchBar;
  QMap<QString, int> m_pageIndexMap;
  
  // Sensor calibration files
  QStringList m_cltSensors;
  QStringList m_iatSensors;
  QStringList m_mapSensors;
  QStringList m_o2Sensors;
  
  // Embedded widgets (formerly separate tabs)
  IdleControlWidget *m_idleWidget;
  ColdStartWidget *m_coldStartWidget;
  AccelEnrichmentWidget *m_accelWidget;
  TrimsWidget *m_trimsWidget;
  VVTVTECWidget *m_vvtVtecWidget;
  BoostWidget *m_boostWidget;
  KnockWidget *m_knockWidget;
  LimitersWidget *m_limitersWidget;
  IOConfigWidget *m_ioConfigWidget;
  
  // Settings Manager for ECU communication
  SerialManager *m_serialManager;
  ECUSettingsManager *m_settingsManager;
  
  // Control-to-setting mappings
  QHash<QString, QComboBox*> m_settingCombos;
  QHash<QString, QSpinBox*> m_settingSpins;
  QHash<QString, QDoubleSpinBox*> m_settingDoubleSpins;
  QHash<QString, QCheckBox*> m_settingChecks;
  QHash<QString, QFrame*> m_ioStatusIndicators; // For BUG-010 live status
  QLabel *m_egoLiveValue = nullptr;  // Live EGO correction display
  
  void loadSensorCalibrationOptions();
  void createEmbeddedWidgets();
  void connectSettingsToECU();
  void initializeSettingsManager();
};

#endif // ECUSETTINGSWIDGET_H
