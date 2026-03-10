#include "HeatmapDelegate.h"
#include "TunerColorMap.h"
#include <QPainter>
#include <QColor>
#include <QRect>

HeatmapDelegate::HeatmapDelegate(double minVal, double maxVal, QObject* parent)
    : QStyledItemDelegate(parent), m_min(minVal), m_max(maxVal) {}

void HeatmapDelegate::setRange(double minVal, double maxVal) {
    m_min = minVal;
    m_max = maxVal;
}

void HeatmapDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                           const QModelIndex& index) const {
    // Save painter state
    painter->save();

    // Get value
    bool ok;
    double value = index.data(Qt::DisplayRole).toDouble(&ok);
    if (!ok) {
        // Fallback for non-numeric data
        QStyledItemDelegate::paint(painter, option, index);
        painter->restore();
        return;
    }

    // Normalize and get background color
    double normalized = TunerColorMap::normalize(value, m_min, m_max);
    QColor bg = TunerColorMap::valueToColor(normalized);

    // Hover effect
    if (option.state & QStyle::State_MouseOver) {
        bg = bg.lighter(115);
    }

    // Fill cell
    painter->fillRect(option.rect, bg);

    // Draw text
    QColor textColor = TunerColorMap::textColorForBackground(bg);
    painter->setPen(textColor);
    painter->setFont(QFont("JetBrains Mono", 12));
    painter->drawText(option.rect, Qt::AlignCenter, QString::number(value, 'f', 2));

    // Selection highlight
    if (option.state & QStyle::State_Selected) {
        painter->setPen(QPen(QColor("#00E5C8"), 2));
        QRect r = option.rect;
        r.adjust(1, 1, -1, -1);
        painter->drawRect(r);
    }

    painter->restore();
}
