#ifndef CIRCULARGAUGE_H
#define CIRCULARGAUGE_H

#include <QWidget>
#include <QPropertyAnimation>
#include <QColor>

class CircularGauge : public QWidget {
    Q_OBJECT
    Q_PROPERTY(double visualValue READ visualValue WRITE setVisualValue)

public:
    explicit CircularGauge(QWidget *parent = nullptr);
    ~CircularGauge();
    
    void setRange(int min, int max);
    void setValue(int value);
    void setLabel(const QString &label);
    
    // Theme setter specifically for the new design
    void setAccentColor(const QColor &color);
    
    double visualValue() const;
    void setVisualValue(double value);

protected:
    void paintEvent(QPaintEvent *event) override;
    
private:
    void drawBackground(QPainter &painter, const QRectF &rect);
    void drawTicks(QPainter &painter, const QRectF &rect);
    void drawArc(QPainter &painter, const QRectF &rect);
    void drawText(QPainter &painter, const QRectF &rect);
    
    int m_minValue;
    int m_maxValue;
    int m_targetValue;
    double m_visualValue;
    QString m_label;
    
    QColor m_accentColor;
    QPropertyAnimation *m_animator;
};

#endif // CIRCULARGAUGE_H
