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
class ECUDefinition;
class LoggingManager;
class LogPlayer;
class ECUSettingsWidget;
class LoggingWidget;

class ECUSettingsManager;
class ToothLoggerWidget;
class ProjectManager;
class UpdateChecker;
class ProjectWizardOverlay;
class ProductTourOverlay;

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
  void onNewProjectClicked();
  void onProjectCreated(const QString &name, const QString &path);
  void onOpenProject(); // Added for ProjectLoader

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
  void paintEvent(QPaintEvent *event) override;
  bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;

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
  LogPlayer *m_logPlayer;
  ECUSettingsManager *m_ecuSettingsManager;
  ProjectManager *m_projectManager; // Added
  UpdateChecker *m_updateChecker;
  QTabWidget *m_tabWidget;

  // Header Widgets
  QPushButton *m_newProjectButton;
  QPushButton *m_projectButton; // Changed from QLabel
  QLabel *m_ecuStatusLabel;
  QPushButton *m_liveTuningButton; // Changed from QLabel to QPushButton
  QPushButton *m_connectButton;
  QPushButton *m_saveButton;
  QComboBox *m_readEcuCombo;

  QPushButton *m_helpButton;

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

  ToothLoggerWidget *m_toothLoggerWidget;

  ProjectWizardOverlay *m_projectWizard;
  ProductTourOverlay *m_tourOverlay = nullptr;

  // State
  bool m_liveTuningEnabled = false;
  
  void startProductTour();
};

#endif // MAINWINDOW_H
