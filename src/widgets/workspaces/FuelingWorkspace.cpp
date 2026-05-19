// D1: FuelingWorkspace implementation.
//
// Each subsection renders settings that mapping.yaml has tagged with this
// workspace + subsection. Editors are built from ECUDefinition::Constant
// metadata (paramClass + bit options + min/max/digits) and bound through
// ECUSettingsManager::setValue, mirroring how ECUSettingsWidget edits.
#include "FuelingWorkspace.h"

#include "core/ECUSettingsManager.h"
#include "core/WorkspaceRegistry.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QSpinBox>
#include <QVBoxLayout>

namespace {

// Build a control bound to a single setting. Returns nullptr if the setting
// has no resolvable definition. The control writes back through
// ECUSettingsManager::setValue with autoWrite=true so changes propagate to
// the ECU as soon as the user edits the value (existing tuning convention).
QWidget* buildControlForSetting(const ECUDefinition::Constant& def,
                                ECUSettingsManager* mgr,
                                QWidget* parent) {
    const QString name = def.name;
    if (def.paramClass == "bits" && !def.bitField.options.isEmpty()) {
        auto* combo = new QComboBox(parent);
        for (const QString& opt : def.bitField.options) combo->addItem(opt);
        const int raw = mgr ? mgr->getRawValue(name) : 0;
        if (raw >= 0 && raw < combo->count()) combo->setCurrentIndex(raw);
        QObject::connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                         [mgr, name](int idx) {
                             if (mgr) mgr->setValue(name, idx, true);
                         });
        return combo;
    }
    if (def.paramClass == "scalar") {
        // Integer if scale = 1 and digits = 0; otherwise double.
        if (def.digits == 0 && def.scale == 1.0) {
            auto* spin = new QSpinBox(parent);
            spin->setMinimum(static_cast<int>(def.min));
            spin->setMaximum(static_cast<int>(def.max ? def.max : 65535));
            spin->setSuffix(def.units.isEmpty() ? QString() : QString(" %1").arg(def.units));
            if (mgr) spin->setValue(mgr->getValue(name).toInt());
            QObject::connect(spin, QOverload<int>::of(&QSpinBox::valueChanged),
                             [mgr, name](int v) {
                                 if (mgr) mgr->setValue(name, v, true);
                             });
            return spin;
        } else {
            auto* spin = new QDoubleSpinBox(parent);
            spin->setMinimum(def.min);
            spin->setMaximum(def.max ? def.max : 1e6);
            spin->setDecimals(def.digits > 0 ? def.digits : 2);
            spin->setSingleStep(def.scale > 0 ? def.scale : 0.1);
            spin->setSuffix(def.units.isEmpty() ? QString() : QString(" %1").arg(def.units));
            if (mgr) spin->setValue(mgr->getValue(name).toDouble());
            QObject::connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                             [mgr, name](double v) {
                                 if (mgr) mgr->setValue(name, v, true);
                             });
            return spin;
        }
    }
    // Arrays, strings, and other classes fall back to a read-only field — the
    // real editor for tables lives in AllTablesWidget; the workspace shows
    // only the entry point.
    auto* line = new QLineEdit(parent);
    line->setReadOnly(true);
    line->setText(mgr ? mgr->getValue(name).toString() : QStringLiteral("(table)"));
    return line;
}

QString humanizeSubsection(const QString& id) {
    // "injector_hardware" -> "Injector Hardware"
    QStringList parts = id.split('_', Qt::SkipEmptyParts);
    for (QString& p : parts) {
        if (!p.isEmpty()) p[0] = p[0].toUpper();
    }
    return parts.join(' ');
}

} // namespace

FuelingWorkspace::FuelingWorkspace(QWidget* parent) : WorkspaceBase(parent) {
    auto* layout = new QVBoxLayout(this);
    auto* header = new QLabel("Fueling", this);
    header->setStyleSheet("color: #ccc; font-size: 18px; font-weight: 500;");
    layout->addWidget(header);

    auto* hint = new QLabel(
        "Subsections render dynamically from mapping.yaml. Open a subsection "
        "from the navigation rail to edit its settings.",
        this);
    hint->setWordWrap(true);
    hint->setStyleSheet("color: #888;");
    layout->addWidget(hint);
    layout->addStretch();
}

void FuelingWorkspace::setSettingsManager(ECUSettingsManager* mgr) {
    m_settingsManager = mgr;
}

QStringList FuelingWorkspace::subsections() const {
    // Discover subsections from the registry: every setting mapped to
    // this workspace contributes its subsection. We also keep the legacy
    // hard-coded list as a fallback for when mapping.yaml is sparse.
    auto& registry = WorkspaceRegistry::instance();
    const QList<QString> mapped = registry.settingsInWorkspace(workspaceId());
    QStringList subs;
    for (const QString& name : mapped) {
        const auto& m = registry.getMapping(name);
        const QString hum = humanizeSubsection(m.subsection);
        if (!hum.isEmpty() && !subs.contains(hum)) subs.append(hum);
    }
    if (subs.isEmpty()) {
        subs = {"Injector Hardware", "Fuel System", "Injection Timing",
                "VE Table", "AFR Target Table", "Acceleration Enrichment",
                "Cold Start Enrichment", "Per-Cylinder Fuel Trims",
                "Closed-Loop EGO"};
    }
    return subs;
}

QWidget* FuelingWorkspace::createSubsectionWidget(const QString& subsectionId) {
    auto& registry = WorkspaceRegistry::instance();

    // Convert the human-readable subsection name back to its underscore form
    // for the registry lookup ("Injector Hardware" -> "injector_hardware").
    const QString subsectionKey =
        subsectionId.toLower().replace(' ', '_');

    auto* container = new QWidget;
    auto* layout    = new QVBoxLayout(container);

    auto* title = new QLabel(QString("<b>%1</b>").arg(subsectionId), container);
    layout->addWidget(title);

    auto* groupBox = new QGroupBox(container);
    auto* form     = new QFormLayout(groupBox);

    const QList<QString> settings =
        registry.settingsInWorkspace(workspaceId(), subsectionKey);

    if (settings.isEmpty()) {
        auto* empty = new QLabel(
            "No settings mapped yet for this subsection. Add entries to "
            "resources/workspaces/mapping.yaml.",
            container);
        empty->setStyleSheet("color: #888;");
        form->addRow(empty);
    } else {
        for (const QString& name : settings) {
            ECUDefinition::Constant def;
            if (m_settingsManager) {
                def = m_settingsManager->getDefinition(name);
            }
            if (def.name.isEmpty()) def.name = name;

            const auto& mapping = registry.getMapping(name);
            const QString label = mapping.humanLabel.isEmpty()
                ? def.name
                : mapping.humanLabel;

            QWidget* editor =
                buildControlForSetting(def, m_settingsManager, container);
            form->addRow(label, editor);
        }
    }

    layout->addWidget(groupBox);
    layout->addStretch();

    // Wrap in a scroll area so dense subsections (e.g. VE table page)
    // remain navigable on small windows.
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setWidget(container);
    return scroll;
}
