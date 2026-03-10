#ifndef COLORLEGENDBAR_H
#define COLORLEGENDBAR_H

#include <QWidget>

class ColorLegendBar : public QWidget {
    Q_OBJECT
public:
    explicit ColorLegendBar(QWidget *parent = nullptr);
    void setRange(double minVal, double maxVal);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    double m_min;
    double m_max;
};

#endif // COLORLEGENDBAR_H
