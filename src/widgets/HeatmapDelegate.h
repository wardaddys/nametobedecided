#ifndef HEATMAPDELEGATE_H
#define HEATMAPDELEGATE_H

#include <QStyledItemDelegate>

class HeatmapDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    HeatmapDelegate(double minVal, double maxVal, QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
               
    void setRange(double minVal, double maxVal);

private:
    double m_min, m_max;
};

#endif // HEATMAPDELEGATE_H
