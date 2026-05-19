#ifndef CURVEEDITORWIDGET_H
#define CURVEEDITORWIDGET_H

#include "core/ECUDefinition.h"
#include <QPolygonF>
#include <QVector>
#include <QWidget>

class QTableWidget;
class ECUSettingsManager;

// E4: Generic 2D curve editor.
//
// Consumes a single ECUDefinition::CurveEditor (populated by the C5 parser)
// and presents:
//   - A grid table for direct X/Y bin editing.
//   - A small SVG-style plot showing the resulting curve.
//
// Used by WUE, AE, knock threshold, MAF flow, IAT correction, idle target,
// cranking PW, and CLT advance editors — anywhere the firmware exposes a 1D
// curve constant.
class CurveEditorWidget : public QWidget {
    Q_OBJECT
public:
    explicit CurveEditorWidget(QWidget* parent = nullptr);

    void setCurve(const ECUDefinition::CurveEditor& curve);
    void setSettingsManager(ECUSettingsManager* mgr);
    void loadFromECU();

protected:
    void paintEvent(QPaintEvent* ev) override;

private slots:
    void onCellChanged(int row, int column);

private:
    void rebuildTable();
    QVector<double> currentY() const;
    QVector<double> currentX() const;

    ECUDefinition::CurveEditor m_curve;
    ECUSettingsManager*        m_settings = nullptr;
    QTableWidget*              m_table    = nullptr;
};

#endif // CURVEEDITORWIDGET_H
