#ifndef TUNERMAP3DWIDGET_H
#define TUNERMAP3DWIDGET_H

#include <QWidget>
#include <QVector>
#include <QColor>
#include <QPoint>
#include <QPointF>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QStringList>
#include <QTimer>

class TunerMap3DWidget : public QWidget {
    Q_OBJECT
public:
    explicit TunerMap3DWidget(QWidget* parent = nullptr);

    void setTableData(const QVector<QVector<double>>& data, double minVal, double maxVal);
    void setData(const QVector<QVector<double>>& data, double minVal, double maxVal) { setTableData(data, minVal, maxVal); }
    void setAxisLabels(const QStringList& xLabels, const QStringList& zLabels);
    void setSelectedIndex(int row, int col) { m_selRow = row; m_selCol = col; update(); }

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;

private slots:
    void toggleSmoothMode();
    void resetView();
    void toggleSync();
    void onAnimTick();

private:
    // ── Data ──────────────────────────────────────────────────────────────────
    QVector<QVector<double>> m_data;
    double m_minVal = 0.0, m_maxVal = 100.0;
    int m_rows = 0, m_cols = 0;
    QStringList m_xLabels, m_zLabels;

    // ── Camera ────────────────────────────────────────────────────────────────
    float   m_azimuth   = 220.0f;
    float   m_elevation = 30.0f;
    float   m_zoom      = 1.0f;
    QPointF m_panOffset = {0, 0};

    // ── Animation (smooth reset) ───────────────────────────────────────────────
    QTimer* m_animTimer    = nullptr;
    float   m_tgtAzimuth   = 220.0f;
    float   m_tgtElevation = 30.0f;

    // ── Interaction ───────────────────────────────────────────────────────────
    QPoint m_lastMouse;
    bool   m_dragging    = false;
    bool   m_smoothMode  = false;
    bool   m_syncEnabled = false;

    // ── Selection / Hover ─────────────────────────────────────────────────────
    int m_selRow = -1, m_selCol = -1;
    int m_hovRow = -1, m_hovCol = -1;

    struct QuadCentre { QPointF screen; int row, col; };
    QVector<QuadCentre> m_centres; // rebuilt each paint, used for hover

    // ── Projection helpers ────────────────────────────────────────────────────
    QPointF project(float x, float y, float z) const;
    float   depth  (float x, float y, float z) const;

    // ── Render passes ─────────────────────────────────────────────────────────
    void renderFloorGrid(QPainter& p);
    void renderSurface  (QPainter& p);
    void renderAxes     (QPainter& p);
    void renderHover    (QPainter& p);
};

#endif // TUNERMAP3DWIDGET_H
