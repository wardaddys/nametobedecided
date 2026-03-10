/**
 * @file SettingsDropdown.h
 * @brief Settings dropdown menu containing Engine Configuration and other
 * settings
 */

#ifndef SETTINGSDROPDOWN_H
#define SETTINGSDROPDOWN_H

#include <QComboBox>
#include <QDialog>
#include <QFrame>
#include <QLabel>
#include <QScrollArea>
#include <QTabWidget>
#include <QVBoxLayout>

class SettingsDropdown : public QDialog {
  Q_OBJECT

public:
  explicit SettingsDropdown(QWidget *parent = nullptr);
  ~SettingsDropdown();

signals:
  // Emitted when valve train setting changes (VTEC, VVT, Standard)
  void vtecSettingChanged(bool enabled);

  // Emitted when engine type changes (NA, Turbo, Supercharged)
  void engineTypeChanged(const QString &type);

  // Emitted when boost control is enabled/disabled
  void boostControlChanged(bool enabled);

protected:
  void showEvent(QShowEvent *event) override;
  bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
  void onValveTrainChanged(int index);
  void onEngineTypeChanged(int index);

private:
  void setupUi();

  // Tab Content Creators
  QWidget *createEngineConfigTab();
  QWidget *createFuelInjectorsTab();
  QWidget *createTechDatabaseTab();
  QWidget *createControlStrategiesTab();

  // Helper to create the styled "badges" (e.g. Green "Boost Control Active")
  QLabel *createBadge(const QString &text, const QString &color);

  // Helper for section headers
  QFrame *createSectionHeader(const QString &icon, const QString &title,
                              QLabel *badge = nullptr);

  QTabWidget *m_tabWidget;

  // Store references to key combo boxes
  QComboBox *m_valveTrainCombo;
  QComboBox *m_engineTypeCombo;
};

#endif // SETTINGSDROPDOWN_H
