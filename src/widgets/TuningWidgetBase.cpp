#include "TuningWidgetBase.h"
#include <QVBoxLayout>

TuningWidgetBase::TuningWidgetBase(QWidget *parent) : QWidget(parent) {
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
}

void TuningWidgetBase::setSettingsManager(ECUSettingsManager *mgr) {
    m_settingsManager = mgr;
}
