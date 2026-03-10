#include "CircularGauge.h"
#include "core/TunerProColors.h"
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>
#include <QtMath>
#include <QFont>
#include <QFontDatabase>
#include <QFontMetrics>

CircularGauge::CircularGauge(QWidget *parent)
    : QWidget(parent), m_minValue(0), m_maxValue(100), m_targetValue(0),
      m_visualValue(0), m_label(""), m_accentColor(QColor(TunerProColors::ACCENT)) {
          
    setMinimumSize(200, 200);
    setMaximumSize(320, 320); // Strict sizing as per spec
    
    // QPropertyAnimation for the visual value
    m_animator = new QPropertyAnimation(this, "visualValue", this);
    m_animator->setDuration(250); // Fast, smooth animation
    m_animator->setEasingCurve(QEasingCurve::OutQuad);
    
    // Global shadow applied to the widget rendering itself. Not quite correct as it affects background.
    // Instead we will draw the glow in the paint event manually or use QGraphicsDropShadowEffect
    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setOffset(0, 0);
    shadow->setColor(QColor(m_accentColor.red(), m_accentColor.green(), m_accentColor.blue(), 75)); // 0.3 opacity glow
    setGraphicsEffect(shadow);
}

CircularGauge::~CircularGauge() {
}

void CircularGauge::setRange(int min, int max) {
    m_minValue = min;
    m_maxValue = max;
    update();
}

void CircularGauge::setLabel(const QString &label) {
    m_label = label;
    update();
}

void CircularGauge::setAccentColor(const QColor &color) {
    m_accentColor = color;
    auto *shadow = static_cast<QGraphicsDropShadowEffect*>(graphicsEffect());
    if (shadow) {
        shadow->setColor(QColor(m_accentColor.red(), m_accentColor.green(), m_accentColor.blue(), 75));
    }
    update();
}

void CircularGauge::setValue(int value) {
    if (m_targetValue == value) return;

    m_targetValue = qBound(m_minValue, value, m_maxValue);
    
    // Animate to new value
    m_animator->stop();
    m_animator->setStartValue(m_visualValue);
    m_animator->setEndValue((double)m_targetValue);
    m_animator->start();
}

double CircularGauge::visualValue() const {
    return m_visualValue;
}

void CircularGauge::setVisualValue(double value) {
    if (qFuzzyCompare(m_visualValue, value)) return;
    m_visualValue = value;
    update();
}

void CircularGauge::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Define working rectangle (square area)
    int side = qMin(width(), height());
    QRectF rect((width() - side) / 2.0, (height() - side) / 2.0, side, side);
    
    // Add margin for drawing elements
    rect.adjust(8, 8, -8, -8);
    
    drawBackground(painter, rect);
    drawTicks(painter, rect);
    drawArc(painter, rect);
    drawText(painter, rect);
}

void CircularGauge::drawBackground(QPainter &painter, const QRectF &rect) {
    // Inner fill
    painter.setBrush(QColor(TunerProColors::BG_RAISED));
    
    // Outer border
    QPen borderPen(QColor(TunerProColors::BORDER_SUBTLE), 2);
    painter.setPen(borderPen);
    
    painter.drawEllipse(rect);
}

void CircularGauge::drawTicks(QPainter &painter, const QRectF &rect) {
    painter.save();
    
    // Move coordinate system to center of gauge
    painter.translate(rect.center());
    
    // Start angle: bottom left (-225 degrees in Qt math context)
    // Actually, visually start is bottom left, so maybe start at 135 deg Cartesian, turning right to 45 deg Cartesian
    // But standard Qt 0 is 3 o'clock, growing CCW.
    // Spec: 270 degree arc. Starting bottom left, going clockwise.
    // Bottom left is ~225 deg. Bottom right is ~315 deg.
    // Qt: 0 is right. We want to start at 225 deg (bottom-left) and sweep -270 deg (clockwise).
    
    // Rotate to start position
    painter.rotate(135.0); // Now 0/360 is at bottom-left
    
    QPen tickPen(QColor(TunerProColors::TEXT_MUTED), 1);
    painter.setPen(tickPen);
    
    double radius = rect.width() / 2.0 - 15.0; // Inset from arc
    
    for (int i = 0; i <= 10; ++i) {
        bool isMajor = (i % 2 == 0); // or (i % 5 == 0)
        double tickLength = isMajor ? 10.0 : 5.0;
        
        QPointF p1(radius - tickLength, 0);
        QPointF p2(radius, 0);
        
        painter.drawLine(p1, p2);
        
        // Rotate for next tick (270 degrees / 10 intervals = 27 degrees per interval)
        painter.rotate(27.0); 
    }
    
    painter.restore();
}

void CircularGauge::drawArc(QPainter &painter, const QRectF &rect) {
    // Inset slightly from the border
    QRectF arcRect = rect.adjusted(12, 12, -12, -12);
    
    // 1. Background Arc
    QPen bgArcPen(QColor(TunerProColors::BORDER_DEFAULT), 8, Qt::SolidLine, Qt::FlatCap);
    painter.setPen(bgArcPen);
    // Qt drawArc uses 1/16th of a degree. Standard CCW from 3 o'clock.
    // Start: 225 deg. Span: -270 deg.
    int startAngle = 225 * 16;
    int spanAngle = -270 * 16;
    painter.drawArc(arcRect, startAngle, spanAngle);
    
    // 2. Value Arc
    QPen valArcPen(m_accentColor, 8, Qt::SolidLine, Qt::FlatCap);
    painter.setPen(valArcPen);
    
    // Calculate span
    double ratio = 0.0;
    if (m_maxValue > m_minValue) {
        ratio = (m_visualValue - m_minValue) / (double)(m_maxValue - m_minValue);
    }
    ratio = qBound(0.0, ratio, 1.0);
    
    int valSpanAngle = -(int)(270 * ratio * 16);
    if (valSpanAngle != 0) {
        painter.drawArc(arcRect, startAngle, valSpanAngle);
    }
}

void CircularGauge::drawText(QPainter &painter, const QRectF &rect) {
    // Draw Value
    QFont valFont("Inter", 56, QFont::Bold);
    valFont.setLetterSpacing(QFont::PercentageSpacing, 96); // -0.04em tracking approx
    painter.setFont(valFont);
    painter.setPen(QColor(TunerProColors::TEXT_PRIMARY));
    
    QString valStr = QString::number((int)m_visualValue);
    
    // We want to center the text in the middle
    QFontMetrics fm(valFont);
    QRect valRect = fm.boundingRect(valStr);
    
    // Draw in the upper-mid section
    QPointF txtPos(rect.center().x() - valRect.width() / 2.0, rect.center().y() + valRect.height() / 4.0);
    painter.drawText(txtPos, valStr);
    
    // Draw Label/Unit
    QFont lblFont("Barlow Condensed", 14, QFont::Normal);
    painter.setFont(lblFont);
    painter.setPen(QColor(TunerProColors::TEXT_MUTED));
    
    QFontMetrics fmLbl(lblFont);
    QRect lblRect = fmLbl.boundingRect(m_label);
    
    QPointF lblPos(rect.center().x() - lblRect.width() / 2.0, rect.center().y() + valRect.height() / 2.0 + 10);
    painter.drawText(lblPos, m_label);
}
