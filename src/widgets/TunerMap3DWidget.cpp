#include "TunerMap3DWidget.h"
#include "TunerColorMap.h"
#include <QPainter>
#include <QPainterPath>
#include <QtMath>
#include <QVector3D>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <algorithm>

TunerMap3DWidget::TunerMap3DWidget(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(320);
    setCursor(Qt::OpenHandCursor);
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // We add a stretch so the toolbar sits at the bottom overlaid or just part of the widget
    layout->addStretch();

    QHBoxLayout* toolbar = new QHBoxLayout();
    toolbar->setContentsMargins(10, 0, 10, 10);
    
    QPushButton* smoothFlatBtn = new QPushButton("SMOOTH / FLAT", this);
    QPushButton* resetViewBtn = new QPushButton("RESET VIEW", this);
    QPushButton* syncSelectionBtn = new QPushButton("SYNC WITH SELECTION", this);

    smoothFlatBtn->setObjectName("smoothFlatBtn");
    resetViewBtn->setObjectName("resetViewBtn");
    syncSelectionBtn->setObjectName("syncSelectionBtn");
    syncSelectionBtn->setCheckable(true);

    QString btnStyle = 
        "QPushButton#smoothFlatBtn, "
        "QPushButton#resetViewBtn, "
        "QPushButton#syncSelectionBtn {"
        "    background: #0F1829;"
        "    color: #7A8FAD;"
        "    border: 1px solid #1A2640;"
        "    padding: 5px 14px;"
        "    font-family: 'JetBrains Mono', monospace;"
        "    font-size: 11px;"
        "    letter-spacing: 0.06em;"
        "}"
        "QPushButton#smoothFlatBtn:hover, "
        "QPushButton#resetViewBtn:hover, "
        "QPushButton#syncSelectionBtn:hover {"
        "    color: #00E5C8;"
        "    border-color: #00E5C860;"
        "}"
        "QPushButton#smoothFlatBtn:checked, "
        "QPushButton#syncSelectionBtn:checked {"
        "    background: #00E5C810;"
        "    color: #00E5C8;"
        "    border-color: #00E5C8;"
        "}";

    smoothFlatBtn->setStyleSheet(btnStyle);
    resetViewBtn->setStyleSheet(btnStyle);
    syncSelectionBtn->setStyleSheet(btnStyle);
    
    toolbar->addWidget(smoothFlatBtn);
    toolbar->addWidget(resetViewBtn);
    toolbar->addWidget(syncSelectionBtn);
    toolbar->addStretch();

    layout->addLayout(toolbar);

    connect(smoothFlatBtn, &QPushButton::clicked, this, &TunerMap3DWidget::toggleSmoothMode);
    connect(resetViewBtn, &QPushButton::clicked, this, &TunerMap3DWidget::resetView);
    connect(syncSelectionBtn, &QPushButton::clicked, this, &TunerMap3DWidget::toggleSync);
}

void TunerMap3DWidget::toggleSmoothMode() {
    m_smoothMode = !m_smoothMode;
    update();
}

void TunerMap3DWidget::resetView() {
    m_azimuth   = 35.0f;
    m_elevation = 28.0f;
    m_zoom      = 1.0f;
    m_panOffset = {0, 0};
    update();
}

void TunerMap3DWidget::toggleSync() {
    m_syncEnabled = !m_syncEnabled;
}

void TunerMap3DWidget::setTableData(const QVector<QVector<double>>& data, double minVal, double maxVal) {
    m_data = data;
    m_minVal = minVal;
    m_maxVal = maxVal;
    m_rows = data.size();
    m_cols = m_rows > 0 ? data[0].size() : 0;
    update();
}

QPointF TunerMap3DWidget::project(float x, float y, float z) const {
    // Rotate by azimuth around Y axis
    float azRad = qDegreesToRadians(m_azimuth);
    float elRad = qDegreesToRadians(m_elevation);

    float rx = x * qCos(azRad) + z * qSin(azRad);
    float ry = y;
    float rz = -x * qSin(azRad) + z * qCos(azRad);

    // Rotate by elevation around X axis
    float ry2 = ry * qCos(elRad) - rz * qSin(elRad);
    float rz2 = ry * qSin(elRad) + rz * qCos(elRad);

    // Perspective divide
    float focalLength = 500.0f * m_zoom;
    float perspective = focalLength / (focalLength + rz2 + 300.0f);

    float screenX = width() / 2.0f  + rx * perspective * 18.0f + m_panOffset.x();
    float screenY = height() / 2.0f - ry2 * perspective * 18.0f + m_panOffset.y();

    return QPointF(screenX, screenY);
}

void TunerMap3DWidget::renderSurface(QPainter& p) {
    if (m_data.isEmpty()) return;

    // Normalize coordinates so the table fits in a -10..+10 cube
    float xScale = 20.0f / qMax(m_cols - 1, 1);
    float zScale = 20.0f / qMax(m_rows - 1, 1);
    float yScale = 12.0f; // height scale

    // Build list of quads with their camera-space Z for sorting
    struct Quad {
        QPointF pts[4];
        QColor  fill;
        float   sortZ;
    };
    QVector<Quad> quads;
    quads.reserve(m_rows * m_cols);

    for (int row = 0; row < m_rows - 1; ++row) {
        for (int col = 0; col < m_cols - 1; ++col) {
            // 4 corners of this cell in 3D space
            double v00 = m_data[row][col];
            double v10 = m_data[row+1][col];
            double v01 = m_data[row][col+1];
            double v11 = m_data[row+1][col+1];

            float x0 = (col)   * xScale - 10.0f;
            float x1 = (col+1) * xScale - 10.0f;
            float z0 = (row)   * zScale - 10.0f;
            float z1 = (row+1) * zScale - 10.0f;

            auto norm = [&](double v) {
                return (v - m_minVal) / qMax(m_maxVal - m_minVal, 0.001);
            };

            float y00 = norm(v00) * yScale - yScale/2;
            float y10 = norm(v10) * yScale - yScale/2;
            float y01 = norm(v01) * yScale - yScale/2;
            float y11 = norm(v11) * yScale - yScale/2;

            // Average color of the 4 corners
            double avgNorm = (norm(v00) + norm(v10) + norm(v01) + norm(v11)) / 4.0;
            QColor fill = TunerColorMap::valueToColor(avgNorm);

            // Simple lighting: compute face normal, dot with light
            QVector3D a(x1-x0, y10-y00, z0-z0);
            QVector3D b(x0-x0, y01-y00, z1-z0);
            QVector3D normal = QVector3D::crossProduct(a, b).normalized();
            QVector3D light(0.5f, 1.0f, 0.5f);
            light.normalize();
            float diffuse = qBound(0.3f, QVector3D::dotProduct(normal, light), 1.0f);

            fill = QColor(
                qBound(0, int(fill.red()   * diffuse), 255),
                qBound(0, int(fill.green() * diffuse), 255),
                qBound(0, int(fill.blue()  * diffuse), 255),
                220
            );

            // Sort key: average Z of the 4 projected points in camera space
            Quad q;
            q.pts[0] = project(x0, y00, z0);
            q.pts[1] = project(x1, y10, z0);
            q.pts[2] = project(x1, y11, z1);
            q.pts[3] = project(x0, y01, z1);
            q.fill   = fill;
            q.sortZ  = (z0 + z1) / 2.0f; // simplified sort key
            quads.append(q);
        }
    }

    // Sort back to front
    std::sort(quads.begin(), quads.end(), [](const Quad& a, const Quad& b) {
        return a.sortZ > b.sortZ;
    });

    // Draw all quads
    p.setRenderHint(QPainter::Antialiasing, m_smoothMode);
    for (const Quad& q : quads) {
        QPolygonF poly;
        for (int i = 0; i < 4; ++i) poly << q.pts[i];

        p.setBrush(q.fill);
        // Edge lines: slightly darker version of fill
        QColor edge;
        if (m_smoothMode) {
            edge = Qt::transparent; // no edges when smooth
        } else {
            edge = q.fill.darker(140);
            edge.setAlpha(180);
        }
        p.setPen(QPen(edge, 0.5f));
        p.drawPolygon(poly);
    }
}

void TunerMap3DWidget::renderFloorGrid(QPainter& p) {
    p.setPen(QPen(QColor(26, 38, 64, 120), 0.5f));
    float yFloor = -(12.0f / 2.0f); // bottom of value range

    for (int i = 0; i <= 10; ++i) {
        float t = i / 10.0f * 20.0f - 10.0f;
        p.drawLine(project(-10, yFloor, t), project(10, yFloor, t));
        p.drawLine(project(t, yFloor, -10), project(t, yFloor, 10));
    }
}

void TunerMap3DWidget::renderAxes(QPainter& p) {
    p.setRenderHint(QPainter::Antialiasing, true);
    QFont font("JetBrains Mono", 9);
    p.setFont(font);

    float yFloor = -6.0f;

    // X axis (RPM)
    p.setPen(QPen(QColor("#00E5C8"), 1.5f));
    p.drawLine(project(-10, yFloor, -10), project(10, yFloor, -10));
    p.setPen(QColor("#7A8FAD"));
    p.drawText(project(12, yFloor, -10).toPoint(), "RPM →");

    // Z axis (Load)
    p.setPen(QPen(QColor("#9B59F5"), 1.5f));
    p.drawLine(project(-10, yFloor, -10), project(-10, yFloor, 10));
    p.setPen(QColor("#7A8FAD"));
    p.drawText(project(-10, yFloor, 12).toPoint(), "Load →");

    // Y axis (Value)
    p.setPen(QPen(QColor("#F5A623"), 1.5f));
    p.drawLine(project(-10, yFloor, -10), project(-10, 6, -10));
    p.setPen(QColor("#7A8FAD"));
    p.drawText(project(-10, 8, -10).toPoint(), "Value");
}

void TunerMap3DWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    // Background
    p.fillRect(rect(), QColor("#03050D"));

    // Subtle radial glow at center
    QRadialGradient glow(width()/2, height()/2, qMin(width(), height()) * 0.6f);
    glow.setColorAt(0, QColor(0, 229, 200, 12));
    glow.setColorAt(1, Qt::transparent);
    p.fillRect(rect(), glow);

    // No data state
    if (m_data.isEmpty()) {
        p.setPen(QColor("#3D5070"));
        p.setFont(QFont("JetBrains Mono", 13));
        p.drawText(rect(), Qt::AlignCenter, "Select a table to view 3D surface");
        return;
    }

    renderFloorGrid(p);
    renderSurface(p);
    renderAxes(p);

    // Overlay: smooth/flat mode indicator top-left
    p.setPen(QColor("#3D5070"));
    p.setFont(QFont("JetBrains Mono", 10));
    p.drawText(QPoint(12, 20), m_smoothMode ? "SMOOTH" : "FLAT");
}

void TunerMap3DWidget::mousePressEvent(QMouseEvent* e) {
    m_lastMousePos = e->pos();
    m_dragging = true;
    setCursor(Qt::ClosedHandCursor);
}

void TunerMap3DWidget::mouseMoveEvent(QMouseEvent* e) {
    if (!m_dragging) return;
    QPoint delta = e->pos() - m_lastMousePos;
    m_lastMousePos = e->pos();

    if (e->buttons() & Qt::LeftButton) {
        m_azimuth   += delta.x() * 0.4f;
        m_elevation += delta.y() * 0.3f;
        m_elevation  = qBound(-89.0f, m_elevation, 89.0f);
    } else if (e->buttons() & Qt::RightButton) {
        m_panOffset += QPointF(delta.x(), delta.y());
    }
    update();
}

void TunerMap3DWidget::mouseReleaseEvent(QMouseEvent*) {
    m_dragging = false;
    setCursor(Qt::OpenHandCursor);
}

void TunerMap3DWidget::wheelEvent(QWheelEvent* e) {
    float delta = e->angleDelta().y() / 120.0f;
    m_zoom = qBound(0.3f, m_zoom + delta * 0.1f, 4.0f);
    update();
}

void TunerMap3DWidget::mouseDoubleClickEvent(QMouseEvent*) {
    m_azimuth   = 35.0f;
    m_elevation = 28.0f;
    m_zoom      = 1.0f;
    m_panOffset = {0, 0};
    update();
}
