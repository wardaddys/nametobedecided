#ifndef REFERENCEDATABASE_H
#define REFERENCEDATABASE_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QList>

struct TechnicalTerm {
    QString id;
    QString category;
    QString title;
    QString shortDescription;
    QString fullDescription;
    QStringList characteristics;
    QStringList tuningFocus;
    QStringList tablesRequired;
    QStringList advantages;
    QStringList disadvantages;
    QMap<QString, QString> typicalValues;
    QStringList relatedTerms;
};

struct InjectorSpec {
    QString id;
    QString brand;
    QString model;
    double flowRateCCMin;
    double flowRateLbHr;
    double impedance;
    double deadTime14V;
    double deadTime12V;
    double deadTime10V;
    double pressure;
    QString category; // street, performance, race, extreme
    QString notes;
    QString manufacturer;
};

class ReferenceDatabase {
public:
    static ReferenceDatabase& instance();

    QList<TechnicalTerm> getTermsByCategory(const QString &category) const;
    TechnicalTerm getTermById(const QString &id) const;
    QStringList getCategories() const;

    QList<InjectorSpec> getAllInjectors() const;
    QList<InjectorSpec> getInjectorsByManufacturer(const QString &manufacturer) const;
    QStringList getManufacturers() const;

private:
    ReferenceDatabase();
    void initializeTechnicalData();
    void initializeInjectorData();

    QMap<QString, TechnicalTerm> m_technicalTerms;
    QList<InjectorSpec> m_injectors;
};

#endif // REFERENCEDATABASE_H
