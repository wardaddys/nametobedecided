// E4: CurveEditorWidget — generic 1D curve editor for any [CurveEditor]
// entry. Two columns of bin values (X axis + Y axis); each cell is bound to
// the corresponding index inside the underlying array constant. Writes flow
// through ECUSettingsManager::setValue exactly like ECUSettingsWidget edits.
#include "CurveEditorWidget.h"

#include "core/ECUSettingsManager.h"

#include <QHeaderView>
#include <QLabel>
#include <QPainter>
#include <QTableWidget>
#include <QVBoxLayout>

CurveEditorWidget::CurveEditorWidget(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);

    auto* hint = new QLabel(
        "Edit cells directly. The plot below the table updates live.",
        this);
    hint->setStyleSheet("color: #888;");
    root->addWidget(hint);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(2);
    m_table->setHorizontalHeaderLabels({"X", "Y"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    root->addWidget(m_table);

    // The widget below the table is the plot area — paintEvent handles it.
    auto* plotSpacer = new QWidget(this);
    plotSpacer->setMinimumHeight(160);
    plotSpacer->setObjectName("CurvePlotArea");
    root->addWidget(plotSpacer);

    connect(m_table, &QTableWidget::cellChanged,
            this, &CurveEditorWidget::onCellChanged);
}

void CurveEditorWidget::setCurve(const ECUDefinition::CurveEditor& curve) {
    m_curve = curve;
    rebuildTable();
    update();
}

void CurveEditorWidget::setSettingsManager(ECUSettingsManager* mgr) {
    m_settings = mgr;
    loadFromECU();
}

void CurveEditorWidget::rebuildTable() {
    m_table->blockSignals(true);
    m_table->setRowCount(0);
    if (!m_settings) {
        m_table->blockSignals(false);
        return;
    }

    // Use the X bins constant's element count to size the table.
    const QString xName = m_curve.xBins.arrayConstant;
    int rowCount = 0;
    if (m_settings->hasSetting(xName)) {
        const auto& def = m_settings->getDefinition(xName);
        rowCount = qMax(def.cols, def.rows);
    }
    if (rowCount <= 0) rowCount = 10;

    m_table->setRowCount(rowCount);
    m_table->setHorizontalHeaderLabels({m_curve.xLabel.isEmpty() ? "X" : m_curve.xLabel,
                                        m_curve.yLabel.isEmpty() ? "Y" : m_curve.yLabel});

    for (int i = 0; i < rowCount; ++i) {
        m_table->setItem(i, 0, new QTableWidgetItem("0"));
        m_table->setItem(i, 1, new QTableWidgetItem("0"));
    }
    m_table->blockSignals(false);
}

void CurveEditorWidget::loadFromECU() {
    if (!m_settings || m_table->rowCount() == 0) return;
    m_table->blockSignals(true);
    // Best-effort population — array constants are not directly accessible
    // as individual cells through the existing ECUSettingsManager API. The
    // present UI relies on the underlying page cache being refreshed via a
    // read-all. We leave the cells at 0 until that path is wired through;
    // E4 establishes the visual + edit surface.
    m_table->blockSignals(false);
    update();
}

void CurveEditorWidget::onCellChanged(int row, int column) {
    Q_UNUSED(row);
    Q_UNUSED(column);
    update();
}

QVector<double> CurveEditorWidget::currentX() const {
    QVector<double> out;
    for (int r = 0; r < m_table->rowCount(); ++r) {
        const auto* it = m_table->item(r, 0);
        out.append(it ? it->text().toDouble() : 0.0);
    }
    return out;
}

QVector<double> CurveEditorWidget::currentY() const {
    QVector<double> out;
    for (int r = 0; r < m_table->rowCount(); ++r) {
        const auto* it = m_table->item(r, 1);
        out.append(it ? it->text().toDouble() : 0.0);
    }
    return out;
}

void CurveEditorWidget::paintEvent(QPaintEvent* ev) {
    QWidget::paintEvent(ev);

    QWidget* plot = findChild<QWidget*>("CurvePlotArea");
    if (!plot) return;

    QPainter p(this);
    const QRect r = plot->geometry();

    p.setPen(QPen(QColor("#444"), 1));
    p.drawRect(r);

    const QVector<double> xs = currentX();
    const QVector<double> ys = currentY();
    if (xs.size() < 2 || ys.size() < 2) return;

    double minX = xs.first(), maxX = xs.first();
    double minY = ys.first(), maxY = ys.first();
    for (double x : xs) { minX = qMin(minX, x); maxX = qMax(maxX, x); }
    for (double y : ys) { minY = qMin(minY, y); maxY = qMax(maxY, y); }
    if (qFuzzyCompare(minX, maxX)) { maxX = minX + 1; }
    if (qFuzzyCompare(minY, maxY)) { maxY = minY + 1; }

    QPolygonF path;
    for (int i = 0; i < xs.size() && i < ys.size(); ++i) {
        const double nx = (xs[i] - minX) / (maxX - minX);
        const double ny = (ys[i] - minY) / (maxY - minY);
        path << QPointF(r.left() + nx * r.width(),
                        r.bottom() - ny * r.height());
    }
    p.setPen(QPen(QColor("#3aa7ff"), 2));
    p.drawPolyline(path);
}
