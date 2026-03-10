#ifndef EXPRESSIONPARSER_H
#define EXPRESSIONPARSER_H

#include <QMap>
#include <QString>
#include <QVariant>
#include <functional>
#include <cmath>

/**
 * @brief Expression Parser for TunerPro
 * 
 * Evaluates expressions from INI files like:
 * - { rpm * 0.1 + 100 }
 * - { condition ? trueVal : falseVal }
 * - { min(rpm, 8000) }
 * - { tableLookup(veTable, rpmBins, mapBins, rpm, map) }
 */
class ExpressionParser {
public:
    using VariableResolver = std::function<double(const QString&)>;
    using ArrayResolver = std::function<QVector<double>(const QString&)>;

    ExpressionParser();

    // Set variable resolver callback (for accessing Constants/OutputChannels)
    void setVariableResolver(VariableResolver resolver);
    void setArrayResolver(ArrayResolver resolver);

    // Parse and evaluate an expression
    double evaluate(const QString& expression);
    
    // Check if a condition expression is true
    bool evaluateCondition(const QString& expression);

    // Parse expression without evaluation (for validation)
    bool validate(const QString& expression);

    // Get last error message
    QString lastError() const { return m_lastError; }

private:
    VariableResolver m_variableResolver;
    ArrayResolver m_arrayResolver;
    QString m_lastError;
    
    // Current parsing state
    QString m_expression;
    int m_pos;
    
    // Recursive descent parser methods
    double parseExpression();
    double parseTernary();
    double parseLogicalOr();
    double parseLogicalAnd();
    double parseBitwiseOr();
    double parseBitwiseXor();
    double parseBitwiseAnd();
    double parseEquality();
    double parseRelational();
    double parseShift();
    double parseAdditive();
    double parseMultiplicative();
    double parseUnary();
    double parsePrimary();
    double parseFunction(const QString& funcName);
    double parseNumber();
    QString parseIdentifier();
    
    // Helper methods
    void skipWhitespace();
    bool match(char c);
    bool match(const QString& s);
    char peek();
    char advance();
    bool isAtEnd();

    // Built-in math functions
    static double fn_sin(double x) { return std::sin(x); }
    static double fn_cos(double x) { return std::cos(x); }
    static double fn_tan(double x) { return std::tan(x); }
    static double fn_asin(double x) { return std::asin(x); }
    static double fn_acos(double x) { return std::acos(x); }
    static double fn_atan(double x) { return std::atan(x); }
    static double fn_sqrt(double x) { return std::sqrt(x); }
    static double fn_abs(double x) { return std::abs(x); }
    static double fn_log(double x) { return std::log(x); }
    static double fn_log10(double x) { return std::log10(x); }
    static double fn_exp(double x) { return std::exp(x); }
    static double fn_pow(double x, double y) { return std::pow(x, y); }
    static double fn_round(double x) { return std::round(x); }
    static double fn_floor(double x) { return std::floor(x); }
    static double fn_ceil(double x) { return std::ceil(x); }
    static double fn_recip(double x) { return 1.0 / x; }
    static double fn_min(double a, double b) { return std::min(a, b); }
    static double fn_max(double a, double b) { return std::max(a, b); }
    static bool fn_isNaN(double x) { return std::isnan(x); }
    
    // Function registry
    QMap<QString, std::function<double(QVector<double>)>> m_functions;
    void registerBuiltinFunctions();
};

#endif // EXPRESSIONPARSER_H
