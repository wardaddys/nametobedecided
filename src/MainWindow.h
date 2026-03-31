#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "core/LoggingManager.h"
#include "core/SerialManager.h"
#include "widgets/DashboardWidget.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>

// Forward declarations — only widgets MainWindow directly owns
class AllTablesWidget;
class ECUSettingsWidget;
class LoggingWidget;
class SettingsDropdown;
class ECUSettingsManager;
class ToothLoggerWidget;
class ProjectManager;
class UpdateChecker;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  void postInit();
  void showEcuDefError();

private slots:
  void onConnectClicked();
  void onSaveClicked();
  void onReadECUChanged(int index);
  void updateRealtimeData(const RealTimeData &data);
  void onConnectionStatusChanged(ConnectionStatus status);

  // Slots
  void onOpenProject(); // Added for ProjectLoader
  void onSettingsClicked();
  void onAboutClicked();
  void onDisconnectClicked();
  void onEnableDemoMode();
  void onDisableDemoMode();
  void onDataReceived(const RealTimeData &data);
  void onError(const QString &error);
  void onEcuSettingsError(const QString &error);
  void onDisconnected();
  void onLiveTuningToggled();

  void onUpdateAvailable(const QString& latestVersion, const QString& downloadUrl, const QString& releaseNotesUrl, bool silentMode);
  void onUpToDate(bool silentMode);
  void onUpdateCheckFailed(const QString& errorMessage, bool silentMode);
  void onManualUpdateCheck();

protected:
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

private:
  QPoint m_dragPosition;
  bool m_dragging = false;

  void setupUi();
  void setupDarkTheme();
  void createCustomTitleBar();
  void createStatusBar();
  void updateConnectionUI(bool connected);

  // Core Components
  SerialManager *m_serialManager;
  LoggingManager *m_loggingManager;
  ECUSettingsManager *m_ecuSettingsManager;
  ProjectManager *m_projectManager; // Added
  UpdateChecker *m_updateChecker;
  QTabWidget *m_tabWidget;

  // Header Widgets
  QPushButton *m_projectButton; // Changed from QLabel
  QLabel *m_ecuStatusLabel;
  QPushButton *m_liveTuningButton; // Changed from QLabel to QPushButton
  QPushButton *m_connectButton;
  QPushButton *m_saveButton;
  QComboBox *m_readEcuCombo;
  QPushButton *m_settingsButton;

  // Status Bar Labels
  QLabel *m_rpmLabel;
  QLabel *m_mapLabel;
  QLabel *m_afrLabel;
  QLabel *m_ectLabel;
  QLabel *m_boostLabel;
  QLabel *m_speedLabel;
  QLabel *m_gearLabel;
  QLabel *m_oilTLabel;
  QLabel *m_oilPLabel;
  QLabel *m_fuelPLabel;
  QLabel *m_ecuSavedLabel;

  // Tab Widgets — only 4 direct children
  DashboardWidget *m_dashboard;
  AllTablesWidget *m_allTablesWidget;
  ECUSettingsWidget *m_ecuSettingsWidget;
  LoggingWidget *m_loggingWidget;
  SettingsDropdown *m_settingsDropdown;
  ToothLoggerWidget *m_toothLoggerWidget;

  // State
  bool m_liveTuningEnabled = false;
};

#endif // MAINWINDOW_H
