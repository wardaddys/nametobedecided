/**
 * @file ProductTourOverlay.h
 * @brief Full-window coachmark overlay for first-run product tour
 *
 * Phase A — v0.5.5: Product Tour
 * A semi-transparent overlay that highlights one widget at a time with a
 * pulsing ring and shows a tooltip with title/body/Next/Skip/Done buttons.
 * Gated by Settings::firstRunCompleted — only shown on first launch.
 */

#ifndef PRODUCTTOUROVERLAY_H
#define PRODUCTTOUROVERLAY_H

#include "TourStep.h"
#include <QWidget>
#include <QVector>
#include <QPropertyAnimation>
#include <QLabel>
#include <QPushButton>

class ProductTourOverlay : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal pulseOpacity READ pulseOpacity WRITE setPulseOpacity)

public:
    explicit ProductTourOverlay(QWidget *parent = nullptr);

    /**
     * @brief Add a step to the tour
     */
    void addStep(const TourStep &step);

    /**
     * @brief Start the tour from step 0
     */
    void startTour();

    /**
     * @brief Returns true if the tour is currently visible/active
     */
    bool isActive() const { return m_active; }

    qreal pulseOpacity() const { return m_pulseOpacity; }
    void  setPulseOpacity(qreal v);

signals:
    void tourCompleted();
    void tourSkipped();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onNextClicked();
    void onSkipClicked();

private:
    void showStep(int index);
    void positionTooltip();
    void finishTour(bool skipped);

    QVector<TourStep> m_steps;
    int  m_currentStep = 0;
    bool m_active = false;

    // Pulse animation
    qreal m_pulseOpacity = 1.0;
    QPropertyAnimation *m_pulseAnim = nullptr;

    // Tooltip widgets (children of this overlay)
    QWidget     *m_tooltipWidget = nullptr;
    QLabel      *m_stepCounter   = nullptr;
    QLabel      *m_titleLabel    = nullptr;
    QLabel      *m_bodyLabel     = nullptr;
    QPushButton *m_nextButton    = nullptr;
    QPushButton *m_skipButton    = nullptr;
};

#endif // PRODUCTTOUROVERLAY_H
