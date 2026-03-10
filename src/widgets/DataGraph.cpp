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
    // Gradient background
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0, m_bgColor.darker(110));
    gradient.setColorAt(1, m_bgColor);
    
    painter.fillRect(rect(), gradient);
    
    // Border
    painter.setPen(QPen(m_gridColor, 1));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
}

void DataGraph::drawGrid(QPainter &painter) {
    int graphWidth = width() - 2 * m_padding;
    int graphHeight = height() - 2 * m_padding;
    int left = m_padding;
    int top = m_padding;
    
    painter.setPen(QPen(m_gridColor, 1, Qt::DotLine));
    
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
    painter.setPen(m_textColor);
    painter.setFont(QFont("Segoe UI", 8));
    
    int bottom = height() - m_padding;
    int top = m_padding;
    
    // Y-axis labels
    double range = m_maxValue - m_minValue;
    for (int i = 0; i <= 4; ++i) {
        double value = m_minValue + (range * (4 - i) / 4);
        int y = top + ((height() - 2 * m_padding) * i / 4);
        
        QString label = QString::number(value, 'f', 1);
        if (!m_unit.isEmpty()) label += " " + m_unit;
        
        painter.drawText(5, y + 4, label);
    }
    
    // Current value (large, right side)
    if (!m_data.isEmpty()) {
        double currentValue = m_data.last();
        painter.setFont(QFont("Segoe UI", 14, QFont::Bold));
        painter.setPen(m_lineColor);
        
        QString valueText = QString::number(currentValue, 'f', 1);
        if (!m_unit.isEmpty()) valueText += " " + m_unit;
        
        int textWidth = painter.fontMetrics().horizontalAdvance(valueText);
        painter.drawText(width() - textWidth - 10, top + 20, valueText);
    }
}

void DataGraph::drawTitle(QPainter &painter) {
    painter.setPen(m_textColor);
    painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
    painter.drawText(m_padding, 15, m_title);
}
