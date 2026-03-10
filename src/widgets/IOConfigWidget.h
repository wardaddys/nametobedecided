#ifndef IOCONFIGWIDGET_H
#define IOCONFIGWIDGET_H

#include <QComboBox>
#include <QGroupBox>
#include <QList>
#include <QStringList>
#include <QTableWidget>
#include <QWidget>

#include "TuningWidgetBase.h"

class IOConfigWidget : public TuningWidgetBase {
  Q_OBJECT

public:
  explicit IOConfigWidget(QWidget *parent = nullptr);
  ~IOConfigWidget() override = default;

  void loadFromECU() override;

private:
  void setupUi();
  void updateConfigFromPreset(int index);

  QComboBox *m_ecuModelCombo;
  QTableWidget *m_analogTable;
  QTableWidget *m_digitalTable;
  QTableWidget *m_digitalOutputTable;

  // Helpers
  QTableWidget *createStyledTable(const QStringList &headers);
  void populateAnalogTable(const QList<QStringList> &data);
  void populateDigitalInputTable(const QList<QStringList> &data);
  void populateDigitalOutputTable(const QList<QStringList> &data);
};

#endif // IOCONFIGWIDGET_H
