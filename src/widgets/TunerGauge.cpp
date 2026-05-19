#include "TunerGauge.h"
#include <QPainter>
#include <QConicalGradient>
#include <QRadialGradient>
#include <QtMath>
#include <QEasingCurve>
#include <QPainterPath>

TunerGauge::TunerGauge(QWidget *parent)
    : QWidget(parent), m_min(0), m_max(100), m_dangerThreshold(85),
      m_value(0), m_visualValue(0), m_peak(0), m_label("GAUGE") {
    
    setAttribute(Qt::WA_TranslucentBackground);
    m_anim = new QPropertyAnimation(this, "visualValue", this);
    m_anim->setDuration(80);
    m_anim->setEasingCurve(QEasingCurve::OutCubic);
}

void TunerGauge::setRange(double minVal, double maxVal) {
    m_min = minVal;
    m_max = maxVal;
    update();
}

void TunerGauge::setLabel(const QString& label) {
    m_label = label;
    update();
}

void TunerGauge::setDangerThreshold(double threshold) {
    m_dangerThreshold = threshold;
    update();
}

void TunerGauge::setValue(double value) {
    if (value > m_peak) m_peak = value;
    m_value = value;
    
    m_anim->stop();
    m_anim->setStartValue(m_visualValue);
    m_anim->setEndValue(value);
    m_anim->start();
}

void TunerGauge::setVisualValue(double v) {
    m_visualValue = v;
    update();
}

QSize TunerGauge::sizeHint() const {
    return QSize(260, 260);
}

void TunerGauge::mousePressEvent(QMouseEvent *event) {
    emit clicked();
    QWidget::mousePressEvent(event);
}

void TunerGauge::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();
    int size = qMin(w, h);
    int cw = w / 2;
    int ch = h / 2;
    
    // Background glow
    QRadialGradient glow(cw, ch, size/2);
    glow.setColorAt(0, QColor(0, 229, 200, 10)); // 0.04 alpha ~ 10/255
    glow.setColorAt(1, Qt::transparent);
    painter.fillRect(rect(), glow);

    painter.translate(cw, ch);

    double r = (size / 2.0) * 0.8;
    
    // Outer ring
    painter.setPen(QPen(QColor("#1A2640"), 8));
    painter.drawArc(QRectF(-r, -r, 2*r, 2*r), 0, 360 * 16);

    double startAngle = 225.0; 
    double sweep = 270.0;
    
    // Danger zone background
    double dangerRatio = (m_max - m_dangerThreshold) / (m_max - m_min);
    if (dangerRatio > 0) {
        double dangerSweep = sweep * dangerRatio;
        int dangerStart = (startAngle - (sweep - dangerSweep)) * 16;
        painter.setPen(QPen(QColor(255, 34, 0, 76), 8)); // #FF220030
        painter.drawArc(QRectF(-r, -r, 2*r, 2*r), dangerStart, -dangerSweep * 16);
    }
    
    double valRatio = qBound(0.0, (m_visualValue - m_min) / (m_max - m_min), 1.0);
    double valSweep = sweep * valRatio;

    // Conical gradient - aligns with arc (225 to -45)
    QConicalGradient cg(0, 0, 315.0); 
    cg.setColorAt(0.0, QColor("#FF2200")); // red at -45 (end)
    cg.setColorAt(135.0/360.0, QColor("#FFB800")); // amber at 90 (middle)
    cg.setColorAt(270.0/360.0, QColor("#00E5C8")); // teal at 225 (start)
    cg.setColorAt(1.0, QColor("#FF2200")); // wrap around
    
    QPen valPen((m_visualValue >= m_dangerThreshold) ? QBrush(QColor("#FF2200")) : QBrush(cg), 8);
    valPen.setCapStyle(Qt::FlatCap);
    if (valSweep > 0) {
        painter.setPen(valPen);
        painter.drawArc(QRectF(-r, -r, 2*r, 2*r), int(startAngle * 16), int(-valSweep * 16));
    }

    // Proportional font sizes
    int tickFontSize = qMax(8, int(r * 0.12));
    int valFontSize  = qMax(16, int(r * 0.40));
    int lblFontSize  = qMax(9, int(r * 0.15));
    int peakFontSize = qMax(8, int(r * 0.12));

    // Ticks
    for (int i = 0; i <= 50; ++i) {
        double tRatio = i / 50.0;
        double angle = startAngle - sweep * tRatio;
        double aRad = qDegreesToRadians(angle);
        
        bool major = (i % 5 == 0);
        double len = major ? 12 : 6;
        double tickR1 = r - 4; // inside arc
        double tickR2 = tickR1 - len;
        
        painter.setPen(QPen(major ? QColor("#7A8FAD") : QColor("#3D5070"), major ? 1.5 : 0.8));
        painter.drawLine(QPointF(tickR1 * cos(aRad), -tickR1 * sin(aRad)),
                         QPointF(tickR2 * cos(aRad), -tickR2 * sin(aRad)));
                         
        if (major) {
            double txtR = r + 15;
            double val = m_min + tRatio * (m_max - m_min);
            painter.setFont(QFont("JetBrains Mono", tickFontSize));
            
            QPointF p(txtR * cos(aRad), -txtR * sin(aRad));
            QRectF txtRect(p.x()-20, p.y()-10, 40, 20);
            painter.drawText(txtRect, Qt::AlignCenter, QString::number(val, 'f', 0));
        }
    }

    // Peak mark
    double peakRatio = qBound(0.0, (m_peak - m_min) / (m_max - m_min), 1.0);
    double peakAngle = startAngle - sweep * peakRatio;
    painter.setPen(QPen(QColor("#FF2200"), 3));
    double peakRad = qDegreesToRadians(peakAngle);
    painter.drawLine(QPointF((r-4)*cos(peakRad), -(r-4)*sin(peakRad)),
                     QPointF((r+4)*cos(peakRad), -(r+4)*sin(peakRad)));

    // Floating Needle (Sexy modern style, does not cross text)
    painter.save();
    painter.rotate(-startAngle + sweep * valRatio + 90);
    
    QPainterPath np;
    np.moveTo(-3, -(r * 0.65));
    np.lineTo(3, -(r * 0.65));
    np.lineTo(1, -(r * 0.85));
    np.lineTo(-1, -(r * 0.85));
    np.closeSubpath();
    
    painter.translate(1, 1);
    painter.fillPath(np, QColor(0,0,0,96)); // Drop shadow
    painter.translate(-1, -1);
    painter.fillPath(np, QColor("#EEF2FF"));
    painter.restore();

    // Readout (Drawn last to ensure it is crisp and clear in the center)
    painter.setPen(QColor("#EEF2FF"));
    painter.setFont(QFont("JetBrains Mono", valFontSize, QFont::Bold));
    painter.drawText(QRectF(-r, -r * 0.4, 2*r, r * 0.8), Qt::AlignCenter, QString::number(m_visualValue, 'f', 0));
    
    painter.setPen(QColor("#7A8FAD"));
    QFont lf("DM Sans", lblFontSize);
    lf.setLetterSpacing(QFont::PercentageSpacing, 110);
    painter.setFont(lf);
    painter.drawText(QRectF(-r, r * 0.35, 2*r, lblFontSize * 2), Qt::AlignCenter, m_label);
    
    painter.setPen(QColor(255, 34, 0, 144));
    painter.setFont(QFont("JetBrains Mono", peakFontSize));
    painter.drawText(QRectF(-r, r * 0.35 + lblFontSize * 1.5, 2*r, peakFontSize * 2), Qt::AlignCenter, QString("PEAK: %1").arg(m_peak, 0, 'f', 0));
}
