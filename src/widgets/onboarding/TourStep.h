/**
 * @file TourStep.h
 * @brief Data struct for a single product tour step
 *
 * Phase A — v0.5.5: Product Tour
 * Each step points at an anchor widget, provides a title/body, and
 * the overlay renders a pulsing highlight ring around the anchor.
 */

#ifndef TOURSTEP_H
#define TOURSTEP_H

#include <QString>
#include <QWidget>
#include <functional>

/**
 * @brief Describes one step in the product tour overlay.
 */
struct TourStep {
    QWidget *anchorWidget = nullptr; ///< Widget to highlight (pulsing ring)
    QString  title;                  ///< Bold headline shown in the tooltip
    QString  body;                   ///< Explanatory text (2-3 sentences max)
    int      padding = 16;           ///< Extra padding around the anchor ring
    std::function<void()> onShow = nullptr; ///< Callback run when this step is shown
};

#endif // TOURSTEP_H
