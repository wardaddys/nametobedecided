/**
 * @file TableEditor.cpp
 * @brief Implementation of TableEditor
 *
 * @author Safeerullah Afridi (FA-22-151)
 * @author Muhammad Saeed Sajid (FA-22-143)
 * @supervisor Prof. Dr. Shariq Hussain
 * @date December 2025
 */

#include "TableEditor.h"
#include "../utils/Logger.h"
#include "../core/TunerProColors.h"
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMimeData>
#include <QtMath>


TableEditor::TableEditor(QWidget *parent)
    : QTableWidget(parent), m_isUpdating(false), m_delegate(nullptr) {
  setupUi();
  m_delegate = new HeatmapDelegate(0.0, 100.0, this);
  setItemDelegate(m_delegate);
  connect(this, &QTableWidget::cellChanged, this, &TableEditor::onCellChanged);
}

void TableEditor::setupUi() {
  // Styling matching Nuclear Spec
  setStyleSheet(QString(R"(
        QTableWidget {
            background-color: %1;
            gridline-color: %2;
            color: %3;
            font-family: 'JetBrains Mono';
            font-size: 14px;
            selection-background-color: %4;
            selection-color: %5;
            border: none;
            border-radius: 0px;
        }
        QTableWidget::item {
            text-align: center;
            border: none;
        }
        QTableWidget::item:selected {
            border: 2px solid %6;
        }
        QHeaderView::section {
            background-color: %7;
            color: %8;
            padding: 4px;
            border: 1px solid %2;
            font-family: 'Inter';
            font-size: 11px;
            font-weight: bold;
        }
        QCornerButton::section {
            background-color: %7;
            border: 1px solid %2;
        }
    )").arg(TunerProColors::BG_BASE)
       .arg(TunerProColors::BORDER_DEFAULT)
       .arg(TunerProColors::TEXT_PRIMARY)
       .arg(TunerProColors::BG_INTERACTIVE)
       .arg(TunerProColors::ACCENT_BRIGHT)
       .arg(TunerProColors::ACCENT)
       .arg(TunerProColors::BG_ELEVATED)
       .arg(TunerProColors::TEXT_SECONDARY));

  setSelectionMode(QAbstractItemView::ContiguousSelection);
  // T6: pixel-perfect section sizes — rows 28px, cols 60px minimum
  horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
  verticalHeader()->setDefaultSectionSize(28);
  horizontalHeader()->setMinimumSectionSize(50);
  verticalHeader()->setMinimumWidth(40);
}

void TableEditor::setTableData(const QVector<QVector<double>> &data,
                               const QStringList &xHeaders,
                               const QStringList &yHeaders) {
  Logger::info(QString("TableEditor::setTableData called. Rows: %1 Cols: %2")
               .arg(data.size()).arg(data.isEmpty() ? 0 : data[0].size()));
  m_isUpdating = true;
  m_data = data;

  setRowCount(yHeaders.size());
  setColumnCount(xHeaders.size());
  setHorizontalHeaderLabels(xHeaders);
  setVerticalHeaderLabels(yHeaders);

  // T2: guard against data/header count mismatch
  int numRows = qMin(data.size(), rowCount());
  for (int r = 0; r < numRows; ++r) {
    int numCols = qMin(data[r].size(), columnCount());
    for (int c = 0; c < numCols; ++c) {
      double val = data[r][c];
      QTableWidgetItem *item =
          new QTableWidgetItem(QString::number(val, 'f', 2));
      item->setTextAlignment(Qt::AlignCenter);
      setItem(r, c, item);
    }
  }
  recalculateMinMax();
  m_isUpdating = false;
  viewport()->update();
}

QVector<QVector<double>> TableEditor::getTableData() const { return m_data; }

void TableEditor::onCellChanged(int row, int col) {
  if (m_isUpdating)
    return;

  QTableWidgetItem *item = this->item(row, col);
  if (!item)
    return;

  bool ok;
  double val = item->text().toDouble(&ok);
  if (ok) {
    if (row < m_data.size() && col < m_data[row].size()) {
      m_data[row][col] = val;
      m_isUpdating = true;
      recalculateMinMax();
      m_isUpdating = false;
      viewport()->update();
      emit dataChanged();
    }
  }
}

void TableEditor::setTableName(const QString& name) {
    m_tableName = name;
}

void TableEditor::recalculateMinMax() {
    double minVal = std::numeric_limits<double>::max();
    double maxVal = std::numeric_limits<double>::lowest();
    
    for (const auto& row : m_data) {
        for (double val : row) {
            if (val < minVal) minVal = val;
            if (val > maxVal) maxVal = val;
        }
    }
    
    if (m_tableName.contains("AFR")) {
        minVal = 10.0;
        maxVal = 20.0;
    } else if (m_tableName.contains("VE")) {
        minVal = 20.0;
        maxVal = 100.0;
    } else if (m_tableName.contains("Ignition")) {
        minVal = 0.0;
        maxVal = 50.0;
    } else {
        if (minVal == std::numeric_limits<double>::max()) {
            minVal = 0.0;
            maxVal = 100.0;
        }
    }
    
    if (m_delegate) {
        m_delegate->setRange(minVal, maxVal);
    }
}

void TableEditor::getRange(double& minVal, double& maxVal) const {
    minVal = 0.0;
    maxVal = 100.0;
    
    if (m_tableName.contains("AFR")) {
        minVal = 10.0;
        maxVal = 20.0;
    } else if (m_tableName.contains("VE")) {
        minVal = 20.0;
        maxVal = 100.0;
    } else if (m_tableName.contains("Ignition")) {
        minVal = 0.0;
        maxVal = 50.0;
    } else {
        minVal = std::numeric_limits<double>::max();
        maxVal = std::numeric_limits<double>::lowest();
        for (const auto& row : m_data) {
            for (double val : row) {
                if (val < minVal) minVal = val;
                if (val > maxVal) maxVal = val;
            }
        }
        if (minVal == std::numeric_limits<double>::max()) {
            minVal = 0.0;
            maxVal = 100.0;
        }
    }
}

void TableEditor::updateCellColor(int row, int col) {
    // Left empty as it's now handled by HeatmapDelegate
}

void TableEditor::adjustSelected(double amount, bool percentage) {
  auto items = selectedItems();
  if (items.isEmpty())
    return;

  m_isUpdating = true;
  for (QTableWidgetItem *item : items) {
    int r = item->row();
    int c = item->column();
    double current = m_data[r][c];

    if (percentage) {
      current *= (1.0 + amount / 100.0);
    } else {
      current += amount;
    }

    m_data[r][c] = current;
    item->setText(QString::number(current, 'f', 2));
  }
  recalculateMinMax();
  m_isUpdating = false;
  viewport()->update();
  emit dataChanged();
}

void TableEditor::smoothSelected() {
  auto items = selectedItems();
  if (items.isEmpty())
    return;

  // Simple 3x3 box blur for selected items
  QVector<QVector<double>> newData = m_data;
  m_isUpdating = true;

  for (QTableWidgetItem *item : items) {
    int r = item->row();
    int c = item->column();

    double sum = 0;
    int count = 0;

    for (int i = -1; i <= 1; ++i) {
      for (int j = -1; j <= 1; ++j) {
        int nr = r + i;
        int nc = c + j;
        if (nr >= 0 && nr < rowCount() && nc >= 0 && nc < columnCount()) {
          sum += m_data[nr][nc];
          count++;
        }
      }
    }

    if (count > 0) {
      newData[r][c] = sum / count;
      item->setText(QString::number(newData[r][c], 'f', 2));
    }
  }

  m_data = newData;
  recalculateMinMax();
  m_isUpdating = false;
  viewport()->update();
  emit dataChanged();
}

void TableEditor::interpolateSelected() {
  // 2D Linear interpolation between corners of selection
  auto ranges = selectedRanges();
  if (ranges.isEmpty())
    return;

  // Only handle first contiguous selection block for simplicity
  QTableWidgetSelectionRange range = ranges.first();
  int top = range.topRow();
  int bottom = range.bottomRow();
  int left = range.leftColumn();
  int right = range.rightColumn();

  if (top == bottom && left == right)
    return;

  m_isUpdating = true;

  double tl = m_data[top][left];
  double tr = m_data[top][right];
  double bl = m_data[bottom][left];
  double br = m_data[bottom][right];

  for (int r = top; r <= bottom; ++r) {
    double vDist = (double)(r - top) / (bottom - top > 0 ? bottom - top : 1);
    for (int c = left; c <= right; ++c) {
      double hDist = (double)(c - left) / (right - left > 0 ? right - left : 1);

      // Bilinear interpolation
      double topVal = tl + (tr - tl) * hDist;
      double botVal = bl + (br - bl) * hDist;
      double finalVal = topVal + (botVal - topVal) * vDist;

      m_data[r][c] = finalVal;
      item(r, c)->setText(QString::number(finalVal, 'f', 2));
    }
  }

  recalculateMinMax();
  m_isUpdating = false;
  viewport()->update();
  emit dataChanged();
}

void TableEditor::copyToClipboard() {
  // T4: TSV export to system clipboard
  QString tsv;
  // Header row (column labels)
  tsv += "\t";
  for (int c = 0; c < columnCount(); ++c) {
    QTableWidgetItem *h = horizontalHeaderItem(c);
    tsv += (h ? h->text() : QString::number(c));
    if (c < columnCount() - 1) tsv += "\t";
  }
  tsv += "\n";

  // Data rows
  for (int r = 0; r < rowCount(); ++r) {
    QTableWidgetItem *rh = verticalHeaderItem(r);
    tsv += (rh ? rh->text() : QString::number(r)) + "\t";
    for (int c = 0; c < columnCount(); ++c) {
      QTableWidgetItem *it = item(r, c);
      tsv += (it ? it->text() : "");
      if (c < columnCount() - 1) tsv += "\t";
    }
    tsv += "\n";
  }

  QMimeData *mime = new QMimeData();
  mime->setText(tsv);
  QApplication::clipboard()->setMimeData(mime);
}

void TableEditor::pasteFromClipboard() {
  // TODO: CSV format paste
}
