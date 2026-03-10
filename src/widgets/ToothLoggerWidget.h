#ifndef TOOTHLOGGERWIDGET_H
#define TOOTHLOGGERWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QVector>
#include <cstdint>

class SerialManager;

class GapVisualizer : public QWidget {
    Q_OBJECT
public:
    explicit GapVisualizer(QWidget *parent = nullptr);
    void setData(const QVector<uint16_t> &data);
    void clearData();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<uint16_t> m_toothTimes;
};

class ToothLoggerWidget : public QWidget {
  Q_OBJECT

public:
  explicit ToothLoggerWidget(QWidget *parent = nullptr);
  void setSerialManager(SerialManager *serial);

public slots:
  void onCaptureClicked();
  void onClearClicked();
  void onToothDataReceived(const QByteArray &data);

private:
  void setupUi();

  SerialManager  *m_serialManager = nullptr;
  QPushButton    *m_captureBtn;
  QPushButton    *m_clearBtn;
  QLabel         *m_statusLabel;
  QLabel         *m_toothCountLabel;
  QTableWidget   *m_toothTable;
  GapVisualizer  *m_gapVisualizer;

  QVector<uint16_t> m_toothTimes;
  bool m_capturing = false;
};

#endif // TOOTHLOGGERWIDGET_H
