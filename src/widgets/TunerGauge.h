#ifndef TUNERGAUGE_H
#define TUNERGAUGE_H

#include <QWidget>
#include <QPropertyAnimation>

class TunerGauge : public QWidget {
    Q_OBJECT
    Q_PROPERTY(double visualValue READ visualValue WRITE setVisualValue)
public:
    explicit TunerGauge(QWidget *parent = nullptr);
    void setRange(double minVal, double maxVal);
    void setLabel(const QString& label);
    void setDangerThreshold(double threshold);
    void setValue(double value);
    QSize sizeHint() const override;

    double visualValue() const { return m_visualValue; }
    void setVisualValue(double v);

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    double m_min;
    double m_max;
    double m_dangerThreshold;
    double m_value;
    double m_visualValue;
    double m_peak;
    QString m_label;
    QPropertyAnimation* m_anim;
};

#endif // TUNERGAUGE_H
