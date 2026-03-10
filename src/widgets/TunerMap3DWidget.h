#ifndef TUNERMAP3DWIDGET_H
#define TUNERMAP3DWIDGET_H

#include <QWidget>
#include <QVector>
#include <QColor>
#include <QPoint>
#include <QPointF>
#include <QMouseEvent>
#include <QWheelEvent>

class TunerMap3DWidget : public QWidget {
    Q_OBJECT
public:
    explicit TunerMap3DWidget(QWidget* parent = nullptr);
    void setTableData(const QVector<QVector<double>>& data, double minVal, double maxVal);

    // Provide a dummy method to keep AllTablesWidget happy, since it might call it
    void setData(const QVector<QVector<double>>& data, double minVal, double maxVal) { setTableData(data, minVal, maxVal); }
    void setSelectedIndex(int row, int col) {
        // Ignored for now based on prompt implementation, but required to build with existing AllTablesWidget
    }

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private slots:
    void toggleSmoothMode();
    void resetView();
    void toggleSync();

private:
    // Data
    QVector<QVector<double>> m_data;
    double m_minVal = 0.0, m_maxVal = 100.0;
    int m_rows = 0, m_cols = 0;

    // Camera state
    float m_azimuth   = 35.0f;   // horizontal rotation degrees
    float m_elevation = 28.0f;   // vertical rotation degrees
    float m_zoom      = 1.0f;
    QPointF m_panOffset = {0, 0};

    // Interaction
    QPoint m_lastMousePos;
    bool m_dragging = false;
    bool m_smoothMode = false;
    bool m_syncEnabled = false;

    // Internal helpers
    QPointF project(float x, float y, float z) const;
    QColor cellColor(double value) const;
    void renderSurface(QPainter& p);
    void renderAxes(QPainter& p);
    void renderFloorGrid(QPainter& p);
};

#endif // TUNERMAP3DWIDGET_H
