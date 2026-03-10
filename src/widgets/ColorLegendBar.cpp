#include "ColorLegendBar.h"
#include "TunerColorMap.h"
#include <QPainter>
#include <QLinearGradient>
#include <QStyleOption>

ColorLegendBar::ColorLegendBar(QWidget *parent)
    : QWidget(parent), m_min(0.0), m_max(100.0) {
    setFixedHeight(16);
}

void ColorLegendBar::setRange(double minVal, double maxVal) {
    m_min = minVal;
    m_max = maxVal;
    update();
}

void ColorLegendBar::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect r = rect();

    QLinearGradient gradient(r.topLeft(), r.topRight());
    gradient.setColorAt(0.0, QColor(10, 22, 40));     // #0A1628
    gradient.setColorAt(0.25, QColor(0, 102, 204));   // #0066CC
    gradient.setColorAt(0.50, QColor(0, 170, 85));    // #00AA55
    gradient.setColorAt(0.75, QColor(255, 184, 0));   // #FFB800
    gradient.setColorAt(1.0, QColor(255, 34, 0));     // #FF2200

    painter.fillRect(r, gradient);

    painter.setPen(Qt::white);
    painter.setFont(QFont("JetBrains Mono", 11));
    
    painter.drawText(r.adjusted(4, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, 
                     QString::number(m_min, 'f', 1));
                     
    painter.drawText(r.adjusted(0, 0, -4, 0), Qt::AlignRight | Qt::AlignVCenter, 
                     QString::number(m_max, 'f', 1));
}
