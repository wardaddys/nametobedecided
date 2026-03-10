/**
 * @file StatusBox.h
 * @brief Status indicator widget
 * 
 * Displays a status label and value/state with a colored indicator.
 * Used for "ECU COMM", "FUEL SYS", etc.
 * 
 * @author Safeerullah Afridi (FA-22-151)
 * @author Muhammad Saeed Sajid (FA-22-143)
 * @supervisor Prof. Dr. Shariq Hussain
 * @date November 2025
 */

#ifndef STATUSBOX_H
#define STATUSBOX_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QFrame>
#include <QColor>

// Forward declaration
class RealTimeData;

class StatusBox : public QWidget {
    Q_OBJECT
    
public:
    explicit StatusBox(const QString &title, QWidget *parent = nullptr);
    
    void setStatus(const QString &status, const QColor &color);
    void setValue(const QString &value);
    
    /**
     * @brief Update status based on ECU data
     * @param data Real-time ECU data
     * 
     * Automatically determines appropriate status and color based on
     * sensor readings and thresholds.
     */
    void updateFromECUData(const RealTimeData &data);
    
private:
    QFrame *m_container;
    QLabel *m_titleLabel;
    QLabel *m_valueLabel;
    QLabel *m_indicator;
};

#endif // STATUSBOX_H
