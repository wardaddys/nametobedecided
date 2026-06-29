#include "DataGraph.h"
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>

DataGraph::DataGraph(QWidget *parent)
    : QWidget(parent)
    , m_title("Data")
    , m_unit("")
    , m_minValue(0)
    , m_maxValue(100)
    , m_maxPoints(100)
    , m_lineColor(0, 255, 128)           // Green
    , m_secondaryLineColor(255, 100, 100) // Red
    , m_gridColor(50, 55, 65)
    , m_bgColor(20, 22, 28)
    , m_textColor(170, 170, 170)
    , m_showGrid(true)
    , m_showLabels(true)
    , m_padding(40)
{
    setMinimumSize(200, 100);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void DataGraph::setTitle(const QString &title) { m_title = title; update(); }
void DataGraph::setRange(double min, double max) { m_minValue = min; m_maxValue = max; update(); }
void DataGraph::setMaxPoints(int points) { m_maxPoints = points; update(); }
void DataGraph::setLineColor(const QColor &color) { m_lineColor = color; update(); }
void DataGraph::setSecondaryLineColor(const QColor &color) { m_secondaryLineColor = color; update(); }
void DataGraph::setGridColor(const QColor &color) { m_gridColor = color; update(); }
void DataGraph::setBackgroundColor(const QColor &color) { m_bgColor = color; update(); }
void DataGraph::setTextColor(const QColor &color) { m_textColor = color; update(); }
void DataGraph::setShowGrid(bool show) { m_showGrid = show; update(); }
void DataGraph::setShowLabels(bool show) { m_showLabels = show; update(); }
void DataGraph::setUnit(const QString &unit) { m_unit = unit; update(); }

void DataGraph::addValue(double value) {
    m_data.append(value);
    while (m_data.size() > m_maxPoints) {
        m_data.removeFirst();
    }
    update();
}

void DataGraph::addSecondaryValue(double value) {
    m_secondaryData.append(value);
    while (m_secondaryData.size() > m_maxPoints) {
        m_secondaryData.removeFirst();
    }
    update();
}

void DataGraph::clear() {
    m_data.clear();
    m_secondaryData.clear();
    update();
}

void DataGraph::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    drawBackground(painter);
    if (m_showGrid) drawGrid(painter);
    drawData(painter);
    if (m_showLabels) drawLabels(painter);
    drawTitle(painter);
}

void DataGraph::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    update();
}

void DataGraph::drawBackground(QPainter &painter) {
    QRect r = rect();
    int bezelWidth = 10;
    
    // 1. Thick 3D Sloped Bezel
    QPoint tl(0, 0), tr(width(), 0), bl(0, height()), br(width(), height());
    QPoint itl(bezelWidth, bezelWidth);
    QPoint itr(width() - bezelWidth, bezelWidth);
    QPoint ibl(bezelWidth, height() - bezelWidth);
    QPoint ibr(width() - bezelWidth, height() - bezelWidth);
    
    painter.setPen(Qt::NoPen);
    
    // Top bezel (lit from above)
    painter.setBrush(QColor(60, 65, 75));
    painter.drawPolygon(QPolygon({tl, tr, itr, itl}));
    
    // Left bezel (ambient light)
    painter.setBrush(QColor(40, 45, 55));
    painter.drawPolygon(QPolygon({tl, itl, ibl, bl}));
    
    // Bottom bezel (shadowed)
    painter.setBrush(QColor(10, 12, 16));
    painter.drawPolygon(QPolygon({bl, ibl, ibr, br}));
    
    // Right bezel (shadowed)
    painter.setBrush(QColor(15, 18, 22));
    painter.drawPolygon(QPolygon({tr, br, ibr, itr}));
    
    // Outer highlight rim
    painter.setPen(QPen(QColor(0, 0, 0), 1));
    painter.drawRect(r.adjusted(0, 0, -1, -1));

    // 2. The Recessed Screen Area
    QRect screenRect(bezelWidth, bezelWidth, width() - 2 * bezelWidth, height() - 2 * bezelWidth);
    
    // CRT Radial Glow
    QRadialGradient bgGlow(screenRect.center(), screenRect.width() / 1.5);
    bgGlow.setColorAt(0, m_bgColor.lighter(130));
    bgGlow.setColorAt(1, m_bgColor.darker(200));
    painter.fillRect(screenRect, bgGlow);
    
    // Corner tube vignette (darkening edges)
    QRadialGradient vignette(screenRect.center(), screenRect.width() / 1.1);
    vignette.setColorAt(0.7, QColor(0, 0, 0, 0));
    vignette.setColorAt(1.0, QColor(0, 0, 0, 220));
    painter.fillRect(screenRect, vignette);
    
    // Scanlines
    painter.setPen(QPen(QColor(0, 0, 0, 60), 1));
    for (int y = screenRect.top(); y < screenRect.bottom(); y += 3) {
        painter.drawLine(screenRect.left(), y, screenRect.right(), y);
    }
    
    // Deep Inner Shadow from the bezel (Top)
    QLinearGradient topShadow(0, screenRect.top(), 0, screenRect.top() + 15);
    topShadow.setColorAt(0, QColor(0, 0, 0, 255));
    topShadow.setColorAt(1, QColor(0, 0, 0, 0));
    painter.fillRect(screenRect.left(), screenRect.top(), screenRect.width(), 15, topShadow);
    
    // Deep Inner Shadow from the bezel (Left)
    QLinearGradient leftShadow(screenRect.left(), 0, screenRect.left() + 15, 0);
    leftShadow.setColorAt(0, QColor(0, 0, 0, 255));
    leftShadow.setColorAt(1, QColor(0, 0, 0, 0));
    painter.fillRect(screenRect.left(), screenRect.top(), 15, screenRect.height(), leftShadow);
    
    // Dramatic Curved Glass Reflection
    QPainterPath glass;
    glass.moveTo(screenRect.topLeft());
    glass.lineTo(screenRect.topRight());
    glass.lineTo(screenRect.right(), screenRect.top() + screenRect.height() * 0.35);
    glass.quadTo(screenRect.center().x(), screenRect.top() + screenRect.height() * 0.55, 
                 screenRect.left(), screenRect.top() + screenRect.height() * 0.35);
    glass.closeSubpath();
    
    QLinearGradient glassGlow(0, screenRect.top(), 0, screenRect.top() + screenRect.height() * 0.5);
    glassGlow.setColorAt(0, QColor(255, 255, 255, 25));
    glassGlow.setColorAt(1, QColor(255, 255, 255, 0));
    painter.fillPath(glass, glassGlow);
}

void DataGraph::drawGrid(QPainter &painter) {
    int graphWidth = width() - 2 * m_padding;
    int graphHeight = height() - 2 * m_padding;
    int left = m_padding;
    int top = m_padding;
    
    // Draw oscilloscope-style solid grid with low opacity matching the phosphor color
    QColor gridColor = m_lineColor; 
    gridColor.setAlpha(30);
    painter.setPen(QPen(gridColor, 1, Qt::SolidLine));
    
    // Horizontal lines (5 divisions)
    for (int i = 0; i <= 4; ++i) {
        int y = top + (graphHeight * i / 4);
        painter.drawLine(left, y, left + graphWidth, y);
    }
    
    // Vertical lines (10 divisions)
    for (int i = 0; i <= 10; ++i) {
        int x = left + (graphWidth * i / 10);
        painter.drawLine(x, top, x, top + graphHeight);
    }
    
    // Center crosshairs slightly brighter
    gridColor.setAlpha(60);
    painter.setPen(QPen(gridColor, 1, Qt::SolidLine));
    painter.drawLine(left, top + graphHeight / 2, left + graphWidth, top + graphHeight / 2);
    painter.drawLine(left + graphWidth / 2, top, left + graphWidth / 2, top + graphHeight);
}

void DataGraph::drawData(QPainter &painter) {
    if (m_data.isEmpty()) return;
    
    int graphWidth = width() - 2 * m_padding;
    int graphHeight = height() - 2 * m_padding;
    int left = m_padding;
    int bottom = height() - m_padding;
    
    double range = m_maxValue - m_minValue;
    if (range <= 0) range = 1;
    
    auto valueToY = [&](double value) -> int {
        double normalized = (value - m_minValue) / range;
        normalized = qBound(0.0, normalized, 1.0);
        return bottom - static_cast<int>(normalized * graphHeight);
    };
    
    double xStep = static_cast<double>(graphWidth) / (m_maxPoints - 1);
    
    // Draw secondary data first (underneath)
    if (!m_secondaryData.isEmpty()) {
        QPainterPath secondaryPath;
        int startIdx = m_maxPoints - m_secondaryData.size();
        
        for (int i = 0; i < m_secondaryData.size(); ++i) {
            int x = left + static_cast<int>((startIdx + i) * xStep);
            int y = valueToY(m_secondaryData[i]);
            
            if (i == 0) {
                secondaryPath.moveTo(x, y);
            } else {
                secondaryPath.lineTo(x, y);
            }
        }
        
        // Draw glow effect
        painter.setPen(QPen(m_secondaryLineColor.lighter(150), 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.setOpacity(0.3);
        painter.drawPath(secondaryPath);
        
        // Draw main line
        painter.setOpacity(1.0);
        painter.setPen(QPen(m_secondaryLineColor, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawPath(secondaryPath);
    }
    
    // Draw primary data
    QPainterPath path;
    int startIdx = m_maxPoints - m_data.size();
    
    for (int i = 0; i < m_data.size(); ++i) {
        int x = left + static_cast<int>((startIdx + i) * xStep);
        int y = valueToY(m_data[i]);
        
        if (i == 0) {
            path.moveTo(x, y);
        } else {
            path.lineTo(x, y);
        }
    }
    
    // Draw glow effect
    painter.setPen(QPen(m_lineColor.lighter(150), 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setOpacity(0.3);
    painter.drawPath(path);
    
    // Draw main line
    painter.setOpacity(1.0);
    painter.setPen(QPen(m_lineColor, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawPath(path);
    
    // Draw fill under line
    if (!m_data.isEmpty()) {
        QPainterPath fillPath = path;
        int lastX = left + static_cast<int>((startIdx + m_data.size() - 1) * xStep);
        int firstX = left + static_cast<int>(startIdx * xStep);
        
        fillPath.lineTo(lastX, bottom);
        fillPath.lineTo(firstX, bottom);
        fillPath.closeSubpath();
        
        QLinearGradient fillGradient(0, m_padding, 0, bottom);
        QColor fillColor = m_lineColor;
        fillColor.setAlpha(60);
        fillGradient.setColorAt(0, fillColor);
        fillColor.setAlpha(10);
        fillGradient.setColorAt(1, fillColor);
        
        painter.fillPath(fillPath, fillGradient);
    }
}

void DataGraph::drawLabels(QPainter &painter) {
    // High contrast labels matching the graph's neon color
    painter.setPen(m_lineColor.lighter(130));
    painter.setFont(QFont("JetBrains Mono", 8, QFont::Bold));
    
    int bottom = height() - m_padding;
    int top = m_padding;
    
    // Y-axis labels
    double range = m_maxValue - m_minValue;
    for (int i = 0; i <= 4; ++i) {
        double value = m_minValue + (range * (4 - i) / 4);
        int y = top + ((height() - 2 * m_padding) * i / 4);
        
        QString label = QString::number(value, 'f', 1);
        if (!m_unit.isEmpty()) label += " " + m_unit;
        
        painter.drawText(16, y + 4, label);
    }
    
    // Current value (large, right side)
    if (!m_data.isEmpty()) {
        double currentValue = m_data.last();
        painter.setFont(QFont("JetBrains Mono", 15, QFont::Bold));
        
        // Draw glow for the text
        painter.setPen(QColor(m_lineColor.red(), m_lineColor.green(), m_lineColor.blue(), 100));
        QString valueText = QString::number(currentValue, 'f', 1);
        if (!m_unit.isEmpty()) valueText += " " + m_unit;
        int textWidth = painter.fontMetrics().horizontalAdvance(valueText);
        painter.drawText(width() - textWidth - 10 + 1, top + 20 + 1, valueText);
        
        // Draw main text
        painter.setPen(m_lineColor);
        painter.drawText(width() - textWidth - 10, top + 20, valueText);
    }
}

void DataGraph::drawTitle(QPainter &painter) {
    painter.setPen(QColor(255, 255, 255, 230)); // High contrast bright white
    painter.setFont(QFont("Inter", 11, QFont::ExtraBold));
    painter.drawText(m_padding, 24, m_title);
}
