/**
 * @file StatusBox.cpp
 * @brief Implementation of StatusBox
 * 
 * @author Safeerullah Afridi (FA-22-151)
 * @author Muhammad Saeed Sajid (FA-22-143)
 * @supervisor Prof. Dr. Shariq Hussain
 * @date November 2025
 */


#include "StatusBox.h"
#include "core/ECUData.h"
#include <QPainter>

StatusBox::StatusBox(const QString &title, QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    m_container = new QFrame(this);
    // MODERN: Glassmorphism effect with gradient background
    m_container->setStyleSheet(
        "QFrame {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 rgba(30, 35, 45, 0.75),"
        "    stop:1 rgba(20, 25, 30, 0.65));"
        "  border: 1px solid rgba(100, 120, 140, 0.4);"
        "  border-top: 1px solid rgba(150, 170, 190, 0.25);"
        "  border-radius: 8px;"
        "}"
    );
    
    QHBoxLayout *frameLayout = new QHBoxLayout(m_container);
    frameLayout->setContentsMargins(10, 5, 10, 5);
    
    // LED Indicator with glow effect
    m_indicator = new QLabel("●", m_container);
    m_indicator->setStyleSheet(
        "color: #444444;"
        "font-size: 20px;"
    );
    m_indicator->setFixedWidth(25);
    
    // Text Container
    QVBoxLayout *textLayout = new QVBoxLayout();
    textLayout->setSpacing(2);
    
    m_titleLabel = new QLabel(title, m_container);
    m_titleLabel->setStyleSheet(
        "color: rgba(180, 190, 200, 0.9);"
        "font-size: 9px;"
        "font-weight: 600;"
        "letter-spacing: 0.5px;"
        "text-transform: uppercase;"
        "font-family: 'Segoe UI', 'Arial';"
    );
    
    m_valueLabel = new QLabel("---", m_container);
    m_valueLabel->setStyleSheet(
        "color: rgba(255, 255, 255, 0.95);"
        "font-size: 13px;"
        "font-weight: bold;"
        "font-family: 'Segoe UI', 'Arial';"
    );
    
    textLayout->addWidget(m_titleLabel);
    textLayout->addWidget(m_valueLabel);
    
    frameLayout->addWidget(m_indicator);
    frameLayout->addLayout(textLayout);
    frameLayout->addStretch();
    
    layout->addWidget(m_container);
}

void StatusBox::setStatus(const QString &text, const QColor &color) {
    m_valueLabel->setText(text);
    
    QString colorName = color.name();
    m_indicator->setStyleSheet(QString("color: %1; font-size: 18px;").arg(colorName));
    m_titleLabel->setStyleSheet(QString("color: %1; font-size: 10px; font-weight: bold; text-transform: uppercase;").arg(colorName));
    
    // Update border color to match status
    m_container->setStyleSheet(QString(
        "background-color: black;"
        "border: 1px solid %1;"
        "border-radius: 4px;"
    ).arg(colorName));
}

void StatusBox::setValue(const QString &value) {
    m_valueLabel->setText(value);
}

/**
 * @brief Update status box based on ECU data
 * 
 * This method automatically determines the appropriate status text and color
 * based on the widget's title and corresponding ECU sensor values.
 */
void StatusBox::updateFromECUData(const RealTimeData &data) {
    QString title = m_titleLabel->text();
    
    // Determine which parameter this status box represents
    if (title.contains("ECU", Qt::CaseInsensitive)) {
        // ECU Communication status - always active if we're getting data
        setStatus("Active", Qt::green);
    }
    else if (title.contains("FUEL", Qt::CaseInsensitive)) {
        // Fuel system status based on corrections
        if (data.egoCorrection > 110 || data.egoCorrection < 90) {
            setStatus("Rich/Lean", QColor(255, 165, 0)); // Orange
        } else {
            setStatus("Normal", Qt::green);
        }
    }
    else if (title.contains("IGNITION", Qt::CaseInsensitive) || title.contains("IGN", Qt::CaseInsensitive)) {
        // Ignition timing
        int timing = data.advance;
        if (timing < 5 || timing > 40) {
            setStatus("Warning", QColor(255, 165, 0)); // Orange
        } else {
            setStatus("Normal", Qt::green);
        }
    }
    else if (title.contains("BOOST", Qt::CaseInsensitive)) {
        // Boost pressure (MAP sensor)
        int mapKpa = data.map;
        double psi = mapKpa * 0.145038 - 14.7; // Convert to boost PSI
        
        if (psi > 0.5) {
            setValue(QString::number(psi, 'f', 1) + " PSI");
            setStatus("", Qt::cyan);
        } else {
            setValue(QString::number(mapKpa) + " kPa");
            setStatus("", Qt::gray);
        }
    }
    else if (title.contains("VTEC", Qt::CaseInsensitive)) {
        // VTEC status (engine bit 0)
        bool vtecActive = (data.engine & 0x01);
        if (vtecActive) {
            setStatus("Engaged", Qt::green);
        } else {
            setStatus("Ready", Qt::gray);
        }
    }
    else if (title.contains("LAUNCH", Qt::CaseInsensitive)) {
        // Launch control (engine bit 1)
        bool launchActive = (data.engine & 0x02);
        if (launchActive) {
            setStatus("ACTIVE", Qt::red);
        } else {
            setStatus("Inactive", QColor(100, 100, 100)); // Dark gray
        }
    }
    else if (title.contains("TRACTION", Qt::CaseInsensitive)) {
        // Traction control
        setStatus("Inactive", QColor(100, 100, 100)); // Not implemented in Speeduino
    }
    else if (title.contains("REV", Qt::CaseInsensitive) || title.contains("LIMIT", Qt::CaseInsensitive)) {
        // Rev limiter status
        if (data.getRPM() > 8500) {
            setStatus("LIMIT HIT", Qt::red);
        } else if (data.getRPM() > 7500) {
            setStatus("Warning", QColor(255, 165, 0)); // Orange
        } else {
            setStatus("Normal", Qt::green);
        }
    }
    else if (title.contains("TEMP", Qt::CaseInsensitive) || title.contains("COOLANT", Qt::CaseInsensitive)) {
        // Coolant temperature
        int temp = data.getCoolant();
        setValue(QString::number(temp) + " °C");
        
        if (temp > 100) {
            setStatus("", Qt::red); // Overheating
        } else if (temp > 95) {
            setStatus("", QColor(255, 165, 0)); // Orange - getting hot
        } else if (temp < 80) {
            setStatus("", Qt::cyan); // Still warming up
        } else {
            setStatus("", Qt::green); // Normal operating temp
        }
    }
    else if (title.contains("AFR", Qt::CaseInsensitive) || title.contains("LAMBDA", Qt::CaseInsensitive)) {
        // Air/Fuel Ratio
        double afr = data.o2 / 10.0;
        setValue(QString::number(afr, 'f', 1));
        
        // Target is typically 14.7 (stoich) or 12.5-13.0 (power)
        if (afr < 12.0 || afr > 15.5) {
            setStatus("", Qt::red); // Dangerously rich or lean
        } else if (afr < 13.5 || afr > 15.0) {
            setStatus("", QColor(255, 165, 0)); // Orange - outside normal
        } else {
            setStatus("", Qt::green);
        }
    }
}

