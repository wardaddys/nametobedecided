#ifndef LOGGINGWIDGET_H
#define LOGGINGWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QListWidget>

class BufferBar : public QWidget {
    Q_OBJECT
    Q_PROPERTY(double value READ value WRITE setValue)
public:
    explicit BufferBar(QWidget *parent = nullptr);
    double value() const { return m_val; }
    void setValue(double v);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    double m_val = 0;
};

class LoggingWidget : public QWidget {
  Q_OBJECT

public:
  explicit LoggingWidget(QWidget *parent = nullptr);
  ~LoggingWidget();

  void setStatus(bool isLogging, const QString &filePath = QString());
  void updateStats(qint64 records, qint64 bytes, double rate);
  void setBufferUsage(int percentage);

signals:
  void startRequested();
  void stopRequested();

private:
  void setupUi();

  // Control Card
  QPushButton *m_toggleButton;
  QLabel *m_statusLabel;
  QLabel *m_fileLabel;
  BufferBar *m_bufferBar;
  
  // Animation
  QPropertyAnimation *m_pulseAnim;
  QGraphicsOpacityEffect *m_pulseEffect;

  // Stats Card
  QLabel *m_recordCountLabel;
  QLabel *m_fileSizeLabel;
  QLabel *m_rateLabel;

  // Recent Logs
  QListWidget *m_recentLogsList;
};

#endif // LOGGINGWIDGET_H
