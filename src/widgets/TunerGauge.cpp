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
    
    // Dynamic Background glow based on value
    QRadialGradient glow(0, 0, size/2);
    QColor glowColor = QColor(0, 229, 200, 15); // Default Teal
    if (m_visualValue >= m_dangerThreshold) {
        glowColor = QColor(255, 34, 0, 30); // Red pulse
    } else if (m_visualValue >= m_max * 0.75) {
        glowColor = QColor(255, 184, 0, 20); // Amber warning
    }
    glow.setColorAt(0, glowColor);
    glow.setColorAt(1, Qt::transparent);
    
    painter.translate(cw, ch);
    painter.fillRect(QRect(-cw, -ch, w, h), glow);
    
    double r = (size / 2.0) * 0.62;

    // Skeuomorphic Gauge Face
    // 1. Strong drop shadow for the housing
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 200));
    painter.drawEllipse(QPointF(4, 6), r + 10, r + 10);

    // 2. Gunmetal / Chrome Outer Bezel
    QConicalGradient bezelGrad(0, 0, 45);
    bezelGrad.setColorAt(0.0, QColor(140, 145, 150));
    bezelGrad.setColorAt(0.25, QColor(40, 45, 50));
    bezelGrad.setColorAt(0.5, QColor(160, 165, 170));
    bezelGrad.setColorAt(0.75, QColor(30, 35, 40));
    bezelGrad.setColorAt(1.0, QColor(140, 145, 150));
    painter.setBrush(bezelGrad);
    painter.drawEllipse(QPointF(0, 0), r + 10, r + 10);

    // Inner rim reflection (deep shadow to light)
    QLinearGradient innerRim(-r, -r, r, r);
    innerRim.setColorAt(0.0, QColor(10, 10, 10)); 
    innerRim.setColorAt(1.0, QColor(120, 125, 130)); 
    painter.setBrush(innerRim);
    painter.drawEllipse(QPointF(0, 0), r + 4, r + 4);

    // 3. Dark Textured Dial Face (slightly convex look)
    QRadialGradient faceGrad(0, 0, r);
    faceGrad.setColorAt(0.0, QColor(45, 48, 55));
    faceGrad.setColorAt(0.8, QColor(20, 22, 25));
    faceGrad.setColorAt(1.0, QColor(10, 12, 15));
    painter.setBrush(faceGrad);
    painter.drawEllipse(QPointF(0, 0), r, r);
    
    // Inset shadow (inner edge)
    QPen insetPen(QColor(0, 0, 0, 255), 3);
    painter.setPen(insetPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(QPointF(0, 0), r, r);

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
            double txtR = r + size * 0.10;
            double val = m_min + tRatio * (m_max - m_min);
            painter.setFont(QFont("JetBrains Mono", tickFontSize, QFont::Bold));
            
            QPointF p(txtR * cos(aRad), -txtR * sin(aRad));
            QRectF txtRect(p.x()-20, p.y()-10, 40, 20);
            
            // Draw cyan glow shadow
            painter.setPen(QColor(0, 229, 200, 70));
            painter.drawText(txtRect.translated(1, 1), Qt::AlignCenter, QString::number(val, 'f', 0));
            
            // Draw main crisp white text
            painter.setPen(QColor(245, 250, 255));
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

    // Dynamic Sweeping Trail & Needle
    painter.save();
    double currentAngle = -startAngle + sweep * valRatio;
    
    // Glowing trail behind the needle
    if (valSweep > 0) {
        QConicalGradient trailGrad(0, 0, currentAngle + 90);
        QColor trailHead = (m_visualValue >= m_dangerThreshold) ? QColor(255, 34, 0, 180) : QColor(0, 229, 200, 180);
        trailGrad.setColorAt(0.0, trailHead);
        trailGrad.setColorAt(0.15, Qt::transparent); // Fades out quickly behind
        trailGrad.setColorAt(1.0, trailHead); // Wrap around safe
        
        QPen trailPen(trailGrad, 12);
        trailPen.setCapStyle(Qt::FlatCap);
        painter.setPen(trailPen);
        painter.drawArc(QRectF(-r+4, -r+4, 2*(r-4), 2*(r-4)), int(startAngle * 16), int(-valSweep * 16));
    }
    
    // The actual needle
    painter.rotate(currentAngle + 90);
    
    // Strong Needle Drop Shadow
    QPainterPath shadowPath;
    shadowPath.moveTo(-2 + 4, -(r * 0.20) + 6);
    shadowPath.lineTo(2 + 4, -(r * 0.20) + 6);
    shadowPath.lineTo(1 + 4, -(r * 0.85) + 6);
    shadowPath.lineTo(-1 + 4, -(r * 0.85) + 6);
    shadowPath.closeSubpath();
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 140));
    painter.drawPath(shadowPath);
    
    // 3D Faceted Needle (Two halves for light/shadow)
    QPainterPath npLight;
    npLight.moveTo(-4, -(r * 0.15));
    npLight.lineTo(0, -(r * 0.15));
    npLight.lineTo(0, -(r * 0.90));
    npLight.lineTo(-1, -(r * 0.90));
    npLight.closeSubpath();
    
    QPainterPath npDark;
    npDark.moveTo(0, -(r * 0.15));
    npDark.lineTo(4, -(r * 0.15));
    npDark.lineTo(1, -(r * 0.90));
    npDark.lineTo(0, -(r * 0.90));
    npDark.closeSubpath();
    
    QColor baseColor = (m_visualValue >= m_dangerThreshold) ? QColor(255, 30, 30) : QColor(255, 70, 0);
    
    painter.setPen(Qt::NoPen);
    painter.setBrush(baseColor.lighter(130)); // Lit side
    painter.drawPath(npLight);
    painter.setBrush(baseColor.darker(150));  // Shadow side
    painter.drawPath(npDark);
    
    // Skeuomorphic Metallic Center Cap (Spun metal)
    double capR = r * 0.18;
    // Drop shadow
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 180));
    painter.drawEllipse(QPointF(2, 3), capR, capR);
    
    // Cap body
    QConicalGradient capGrad(0, 0, 135);
    capGrad.setColorAt(0.0, QColor(120, 125, 130));
    capGrad.setColorAt(0.25, QColor(40, 45, 50));
    capGrad.setColorAt(0.5, QColor(140, 145, 150));
    capGrad.setColorAt(0.75, QColor(30, 35, 40));
    capGrad.setColorAt(1.0, QColor(120, 125, 130));
    painter.setBrush(capGrad);
    painter.setPen(QPen(QColor(15, 15, 15), 1));
    painter.drawEllipse(QPointF(0, 0), capR, capR);
    
    // Cap indent/detail
    QLinearGradient indentGrad(-capR*0.5, -capR*0.5, capR*0.5, capR*0.5);
    indentGrad.setColorAt(0, QColor(20, 20, 20));
    indentGrad.setColorAt(1, QColor(80, 80, 80));
    painter.setBrush(indentGrad);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPointF(0, 0), capR*0.5, capR*0.5);

    // Glassmorphic Inner Cover (Specular Highlight)
    QRadialGradient glassHighlight(0, -r*0.6, r*1.1);
    glassHighlight.setColorAt(0, QColor(255, 255, 255, 40)); // Stronger white highlight
    glassHighlight.setColorAt(0.4, QColor(255, 255, 255, 5));
    glassHighlight.setColorAt(1, Qt::transparent);
    painter.setPen(Qt::NoPen);
    painter.setBrush(glassHighlight);
    painter.drawEllipse(QPointF(0, 0), r, r);

    
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
