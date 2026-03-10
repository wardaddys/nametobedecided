#ifndef DATAGRAPH_H
#define DATAGRAPH_H

#include <QWidget>
#include <QVector>
#include <QColor>
#include <QTimer>

/**
 * @brief Real-time data graph widget
 * 
 * Displays scrolling line chart for real-time data visualization.
 * Shows data history with configurable colors and range.
 */
class DataGraph : public QWidget {
    Q_OBJECT

public:
    explicit DataGraph(QWidget *parent = nullptr);
    ~DataGraph() = default;

    // Configuration
    void setTitle(const QString &title);
    void setRange(double min, double max);
    void setMaxPoints(int points);
    void setLineColor(const QColor &color);
    void setSecondaryLineColor(const QColor &color);
    void setGridColor(const QColor &color);
    void setBackgroundColor(const QColor &color);
    void setTextColor(const QColor &color);
    void setShowGrid(bool show);
    void setShowLabels(bool show);
    void setUnit(const QString &unit);

    // Data
    void addValue(double value);
    void addSecondaryValue(double value);
    void clear();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void drawBackground(QPainter &painter);
    void drawGrid(QPainter &painter);
    void drawData(QPainter &painter);
    void drawLabels(QPainter &painter);
    void drawTitle(QPainter &painter);

    QString m_title;
    QString m_unit;
    double m_minValue;
    double m_maxValue;
    int m_maxPoints;
    
    QVector<double> m_data;
    QVector<double> m_secondaryData;
    
    QColor m_lineColor;
    QColor m_secondaryLineColor;
    QColor m_gridColor;
    QColor m_bgColor;
    QColor m_textColor;
    
    bool m_showGrid;
    bool m_showLabels;
    
    int m_padding;
};

#endif // DATAGRAPH_H
