#ifndef INJECTORVISUALWIDGET_H
#define INJECTORVISUALWIDGET_H

#include <QWidget>
#include <QColor>

/**
 * @brief 3D injector nozzle widget for the Duty % panel.
 *
 * Renders a skeuomorphic cross-section of a fuel injector tip with:
 *  - Machined steel injector body and pintle
 *  - Dynamic fuel spray cone that widens and intensifies with duty cycle
 *  - Glowing blue/cyan fuel mist effect
 *  - Digital Duty % readout
 *
 * Call setDuty(double pct) to drive the animation.
 * Core functionality is untouched — this is purely a QPainter widget.
 */
class InjectorVisualWidget : public QWidget {
    Q_OBJECT
public:
    explicit InjectorVisualWidget(QWidget *parent = nullptr);

    /** Set the current Duty Cycle % (range 0–100). Triggers repaint. */
    void setDuty(double pct);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    double m_duty = 0.0;
};

#endif // INJECTORVISUALWIDGET_H
