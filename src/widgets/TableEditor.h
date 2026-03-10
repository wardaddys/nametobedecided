/**
 * @file TableEditor.h
 * @brief Reusable Table Editor Widget
 *
 * @author Safeerullah Afridi (FA-22-151)
 * @author Muhammad Saeed Sajid (FA-22-143)
 * @supervisor Prof. Dr. Shariq Hussain
 * @date December 2025
 */

#ifndef TABLEEDITOR_H
#define TABLEEDITOR_H

#include <QTableWidget>
#include <QVector>
#include <QWidget>
#include "HeatmapDelegate.h"

class TableEditor : public QTableWidget {
  Q_OBJECT

public:
  explicit TableEditor(QWidget *parent = nullptr);

  // Data management
  void setTableData(const QVector<QVector<double>> &data,
                    const QStringList &xHeaders, const QStringList &yHeaders);
  QVector<QVector<double>> getTableData() const;
  
  void setTableName(const QString& name);
  void recalculateMinMax();
  void getRange(double& minVal, double& maxVal) const;

  // Visuals
  void applyGradient(const QColor &lowColor, const QColor &highColor);
  void highlightCell(int row, int col);

  // Operations
  void adjustSelected(double amount, bool percentage = false);
  void smoothSelected();
  void interpolateSelected();
  void copyToClipboard();
  void pasteFromClipboard();

signals:
  void dataChanged();
  void cellSelectionChanged();

private slots:
  void onCellChanged(int row, int col);

private:
  void setupUi();
  void updateCellColor(int row, int col);

  QVector<QVector<double>> m_data;
  bool m_isUpdating;
  QString m_tableName;
  HeatmapDelegate *m_delegate;
};

#endif // TABLEEDITOR_H
