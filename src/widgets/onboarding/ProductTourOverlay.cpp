/**
 * @file ProductTourOverlay.cpp
 * @brief Implementation of the product tour coachmark overlay
 *
 * Phase A — v0.5.5: Product Tour
 * Renders a dimmed overlay with a cutout around the anchor widget,
 * a pulsing ring animation, and a floating tooltip card.
 */

#include "ProductTourOverlay.h"
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QEvent>
#include <QResizeEvent>

ProductTourOverlay::ProductTourOverlay(QWidget *parent)
    : QWidget(parent)
{
    // Cover the entire parent
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setFocusPolicy(Qt::StrongFocus);

    // --- Build the floating tooltip card ---
    m_tooltipWidget = new QWidget(this);
    m_tooltipWidget->setObjectName("TourTooltip");
    m_tooltipWidget->setFixedWidth(380);
    m_tooltipWidget->setStyleSheet(
        "#TourTooltip {"
        "  background: #1e1e2e;"
        "  border: 1px solid #45475a;"
        "  border-radius: 12px;"
        "  padding: 0px;"
        "}"
    );

    // Drop shadow on the tooltip
    auto *shadow = new QGraphicsDropShadowEffect(m_tooltipWidget);
    shadow->setBlurRadius(32);
    shadow->setColor(QColor(0, 0, 0, 160));
    shadow->setOffset(0, 8);
    m_tooltipWidget->setGraphicsEffect(shadow);

    auto *tooltipLayout = new QVBoxLayout(m_tooltipWidget);
    tooltipLayout->setContentsMargins(24, 20, 24, 20);
    tooltipLayout->setSpacing(12);

    // Step counter (e.g., "Step 1 of 6")
    m_stepCounter = new QLabel(this);
    m_stepCounter->setStyleSheet(
        "color: #89b4fa; font-size: 11px; font-weight: 600; letter-spacing: 1px;"
    );
    tooltipLayout->addWidget(m_stepCounter);

    // Title
    m_titleLabel = new QLabel(this);
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setStyleSheet(
        "color: #cdd6f4; font-size: 16px; font-weight: 700;"
    );
    tooltipLayout->addWidget(m_titleLabel);

    // Body
    m_bodyLabel = new QLabel(this);
    m_bodyLabel->setWordWrap(true);
    m_bodyLabel->setStyleSheet(
        "color: #a6adc8; font-size: 13px; line-height: 1.5;"
    );
    tooltipLayout->addWidget(m_bodyLabel);

    tooltipLayout->addSpacing(8);

    // Button row
    auto *buttonRow = new QHBoxLayout();
    buttonRow->setSpacing(12);

    m_skipButton = new QPushButton("Skip Tour", this);
    m_skipButton->setCursor(Qt::PointingHandCursor);
    m_skipButton->setStyleSheet(
        "QPushButton {"
        "  background: transparent; color: #6c7086; border: none;"
        "  font-size: 12px; padding: 8px 16px;"
        "}"
        "QPushButton:hover { color: #a6adc8; }"
    );

    m_nextButton = new QPushButton("Next →", this);
    m_nextButton->setCursor(Qt::PointingHandCursor);
    m_nextButton->setStyleSheet(
        "QPushButton {"
        "  background: #89b4fa; color: #1e1e2e; border: none;"
        "  border-radius: 6px; font-size: 13px; font-weight: 600;"
        "  padding: 8px 24px;"
        "}"
        "QPushButton:hover { background: #b4d0fb; }"
    );

    buttonRow->addWidget(m_skipButton);
    buttonRow->addStretch();
    buttonRow->addWidget(m_nextButton);
    tooltipLayout->addLayout(buttonRow);

    connect(m_nextButton, &QPushButton::clicked, this, &ProductTourOverlay::onNextClicked);
    connect(m_skipButton, &QPushButton::clicked, this, &ProductTourOverlay::onSkipClicked);

    // --- Pulse animation (2-second cycle, 0.4 ↔ 1.0 opacity) ---
    m_pulseAnim = new QPropertyAnimation(this, "pulseOpacity", this);
    m_pulseAnim->setDuration(2000);
    m_pulseAnim->setStartValue(1.0);
    m_pulseAnim->setEndValue(0.4);
    m_pulseAnim->setEasingCurve(QEasingCurve::InOutSine);
    m_pulseAnim->setLoopCount(-1); // infinite
    // Reverse on each loop for smooth breathing effect
    connect(m_pulseAnim, &QPropertyAnimation::currentLoopChanged, this, [this]() {
        if (m_pulseAnim->direction() == QAbstractAnimation::Forward)
            m_pulseAnim->setDirection(QAbstractAnimation::Backward);
        else
            m_pulseAnim->setDirection(QAbstractAnimation::Forward);
    });

    // Start hidden
    hide();
    m_tooltipWidget->hide();
}

void ProductTourOverlay::addStep(const TourStep &step) {
    m_steps.append(step);
}

void ProductTourOverlay::startTour() {
    if (m_steps.isEmpty()) return;

    m_active = true;
    m_currentStep = 0;

    // Install event filter on parent to track resizes
    if (parentWidget()) {
        parentWidget()->installEventFilter(this);
        resize(parentWidget()->size());
    }

    show();
    raise();
    m_tooltipWidget->show();
    m_tooltipWidget->raise();
    m_pulseAnim->start();

    showStep(0);
}

void ProductTourOverlay::showStep(int index) {
    if (index < 0 || index >= m_steps.size()) return;

    m_currentStep = index;
    const TourStep &step = m_steps[index];

    // Update text
    m_stepCounter->setText(QString("STEP %1 OF %2").arg(index + 1).arg(m_steps.size()));
    m_titleLabel->setText(step.title);
    m_bodyLabel->setText(step.body);

    // Update button text
    if (index == m_steps.size() - 1) {
        m_nextButton->setText("Done ✓");
    } else {
        m_nextButton->setText("Next →");
    }

    // Execute step callback if provided (e.g. to switch tabs before highlighting)
    if (step.onShow) {
        step.onShow();
    }

    positionTooltip();
    update(); // trigger repaint for the highlight ring
}

void ProductTourOverlay::positionTooltip() {
    if (m_currentStep < 0 || m_currentStep >= m_steps.size()) return;

    const TourStep &step = m_steps[m_currentStep];
    QWidget *anchor = step.anchorWidget;

    if (!anchor || !anchor->isVisible()) {
        // No anchor or not visible — center the tooltip
        int tx = (width() - m_tooltipWidget->width()) / 2;
        int ty = height() / 2 - 100;
        m_tooltipWidget->move(tx, ty);
        return;
    }

    // Get anchor rect in our coordinate space
    QPoint anchorTopLeft = anchor->mapTo(this, QPoint(0, 0));
    QRect anchorRect(anchorTopLeft, anchor->size());
    int pad = step.padding;

    // Preferred position: below the anchor, centered horizontally
    int tx = anchorRect.center().x() - m_tooltipWidget->width() / 2;
    int ty = anchorRect.bottom() + pad + 12;

    // If tooltip would go off the bottom, put it above
    if (ty + m_tooltipWidget->sizeHint().height() > height() - 20) {
        ty = anchorRect.top() - pad - m_tooltipWidget->sizeHint().height() - 12;
    }

    // Clamp horizontally
    if (tx < 20) tx = 20;
    if (tx + m_tooltipWidget->width() > width() - 20) {
        tx = width() - m_tooltipWidget->width() - 20;
    }

    // Clamp vertically
    if (ty < 20) ty = 20;

    m_tooltipWidget->move(tx, ty);
    m_tooltipWidget->adjustSize();
}

void ProductTourOverlay::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 1. Dimmed background covering everything
    QPainterPath fullPath;
    fullPath.addRect(rect());

    QPainterPath cutoutPath;

    // 2. Cut out the anchor widget area (if it exists and is visible)
    if (m_currentStep >= 0 && m_currentStep < m_steps.size()) {
        const TourStep &step = m_steps[m_currentStep];
        QWidget *anchor = step.anchorWidget;

        if (anchor && anchor->isVisible()) {
            QPoint anchorTopLeft = anchor->mapTo(this, QPoint(0, 0));
            QRect anchorRect(anchorTopLeft, anchor->size());
            int pad = step.padding;
            QRect highlightRect = anchorRect.adjusted(-pad, -pad, pad, pad);

            // Rounded cutout
            cutoutPath.addRoundedRect(highlightRect, 12, 12);

            // Draw the overlay with cutout
            QPainterPath dimPath = fullPath - cutoutPath;
            painter.fillPath(dimPath, QColor(0, 0, 0, 160));

            // 3. Pulsing ring around the cutout
            QPen ringPen(QColor(137, 180, 250, static_cast<int>(m_pulseOpacity * 255)));
            ringPen.setWidth(3);
            painter.setPen(ringPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawRoundedRect(highlightRect, 12, 12);

            // Outer glow ring
            QPen glowPen(QColor(137, 180, 250, static_cast<int>(m_pulseOpacity * 80)));
            glowPen.setWidth(6);
            painter.setPen(glowPen);
            QRect outerGlow = highlightRect.adjusted(-4, -4, 4, 4);
            painter.drawRoundedRect(outerGlow, 14, 14);
        } else {
            // No anchor — just dim everything
            painter.fillPath(fullPath, QColor(0, 0, 0, 160));
        }
    } else {
        painter.fillPath(fullPath, QColor(0, 0, 0, 160));
    }
}

void ProductTourOverlay::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    if (m_active) {
        positionTooltip();
    }
}

bool ProductTourOverlay::eventFilter(QObject *obj, QEvent *event) {
    if (obj == parentWidget() && event->type() == QEvent::Resize) {
        resize(parentWidget()->size());
        positionTooltip();
    }
    return QWidget::eventFilter(obj, event);
}

void ProductTourOverlay::setPulseOpacity(qreal v) {
    m_pulseOpacity = v;
    update();
}

void ProductTourOverlay::onNextClicked() {
    if (m_currentStep < m_steps.size() - 1) {
        showStep(m_currentStep + 1);
    } else {
        // Last step — tour complete
        finishTour(false);
    }
}

void ProductTourOverlay::onSkipClicked() {
    finishTour(true);
}

void ProductTourOverlay::finishTour(bool skipped) {
    m_active = false;
    m_pulseAnim->stop();
    m_tooltipWidget->hide();
    hide();

    if (parentWidget()) {
        parentWidget()->removeEventFilter(this);
    }

    if (skipped) {
        emit tourSkipped();
    } else {
        emit tourCompleted();
    }
}
