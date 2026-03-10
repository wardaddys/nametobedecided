#include "ExpressionParser.h"
#include <QDebug>
#include <QRegularExpression>

ExpressionParser::ExpressionParser()
    : m_pos(0)
{
    registerBuiltinFunctions();
}

void ExpressionParser::setVariableResolver(VariableResolver resolver) {
    m_variableResolver = resolver;
}

void ExpressionParser::setArrayResolver(ArrayResolver resolver) {
    m_arrayResolver = resolver;
}

void ExpressionParser::registerBuiltinFunctions() {
    // Single argument functions
    m_functions["sin"] = [](QVector<double> args) { return args.size() >= 1 ? std::sin(args[0]) : 0; };
    m_functions["cos"] = [](QVector<double> args) { return args.size() >= 1 ? std::cos(args[0]) : 0; };
    m_functions["tan"] = [](QVector<double> args) { return args.size() >= 1 ? std::tan(args[0]) : 0; };
    m_functions["asin"] = [](QVector<double> args) { return args.size() >= 1 ? std::asin(args[0]) : 0; };
    m_functions["acos"] = [](QVector<double> args) { return args.size() >= 1 ? std::acos(args[0]) : 0; };
    m_functions["atan"] = [](QVector<double> args) { return args.size() >= 1 ? std::atan(args[0]) : 0; };
    m_functions["sqrt"] = [](QVector<double> args) { return args.size() >= 1 ? std::sqrt(args[0]) : 0; };
    m_functions["abs"] = [](QVector<double> args) { return args.size() >= 1 ? std::abs(args[0]) : 0; };
    m_functions["log"] = [](QVector<double> args) { return args.size() >= 1 ? std::log(args[0]) : 0; };
    m_functions["log10"] = [](QVector<double> args) { return args.size() >= 1 ? std::log10(args[0]) : 0; };
    m_functions["exp"] = [](QVector<double> args) { return args.size() >= 1 ? std::exp(args[0]) : 0; };
    m_functions["round"] = [](QVector<double> args) { return args.size() >= 1 ? std::round(args[0]) : 0; };
    m_functions["floor"] = [](QVector<double> args) { return args.size() >= 1 ? std::floor(args[0]) : 0; };
    m_functions["ceil"] = [](QVector<double> args) { return args.size() >= 1 ? std::ceil(args[0]) : 0; };
    m_functions["recip"] = [](QVector<double> args) { return args.size() >= 1 ? 1.0 / args[0] : 0; };
    m_functions["isNaN"] = [](QVector<double> args) { return args.size() >= 1 ? (std::isnan(args[0]) ? 1.0 : 0.0) : 0; };
    
    // Two argument functions
    m_functions["pow"] = [](QVector<double> args) { 
        return args.size() >= 2 ? std::pow(args[0], args[1]) : 0; 
    };
    m_functions["min"] = [](QVector<double> args) { 
        if (args.isEmpty()) return 0.0;
        double result = args[0];
        for (int i = 1; i < args.size(); ++i) {
            result = std::min(result, args[i]);
        }
        return result;
    };
    m_functions["max"] = [](QVector<double> args) { 
        if (args.isEmpty()) return 0.0;
        double result = args[0];
        for (int i = 1; i < args.size(); ++i) {
            result = std::max(result, args[i]);
        }
        return result;
    };
    m_functions["sum"] = [](QVector<double> args) { 
        double result = 0;
        for (double v : args) result += v;
        return result;
    };
    
    // Conditional if(condition, then, else)
    m_functions["if"] = [](QVector<double> args) { 
        if (args.size() >= 3) {
            return args[0] != 0 ? args[1] : args[2];
        }
        return 0.0;
    };

    // selectExpression(index, expr0, expr1, expr2, ...)
    m_functions["selectExpression"] = [](QVector<double> args) {
        if (args.size() < 2) return 0.0;
        int index = static_cast<int>(args[0]);
        if (index < 0 || index >= args.size() - 1) return 0.0;
        return args[index + 1];
    };

    // arrayValue(array, index) - gets interpolated value
    m_functions["arrayValue"] = [this](QVector<double> args) {
        // This requires array resolver - simplified version
        if (args.size() >= 2) {
            // For now, just return the second arg as index
            return args[1];
        }
        return 0.0;
    };

    // smoothBasic(field, smoothingFactor)
    m_functions["smoothBasic"] = [](QVector<double> args) {
        // Simplified - would need history for real smoothing
        return args.size() >= 1 ? args[0] : 0;
    };

    // lastValue - returns value from previous record
    m_functions["lastValue"] = [](QVector<double> args) {
        return args.size() >= 1 ? args[0] : 0;
    };

    // timeNow - returns current time in seconds (simplified)
    m_functions["timeNow"] = [](QVector<double>) {
        return 0.0; // Would need timer integration
    };

    // isOnline - returns 1 if connected
    m_functions["isOnline"] = [](QVector<double>) {
        return 0.0; // Would need connection state
    };
}

double ExpressionParser::evaluate(const QString& expression) {
    m_lastError.clear();
    
    // Strip leading/trailing braces if present
    QString expr = expression.trimmed();
    if (expr.startsWith('{') && expr.endsWith('}')) {
        expr = expr.mid(1, expr.length() - 2).trimmed();
    }
    
    if (expr.isEmpty()) {
        return 0.0;
    }
    
    m_expression = expr;
    m_pos = 0;
    
    try {
        double result = parseExpression();
        skipWhitespace();
        if (!isAtEnd()) {
            m_lastError = QString("Unexpected character at position %1").arg(m_pos);
        }
        return result;
    } catch (const std::exception& e) {
        m_lastError = QString("Parse error: %1").arg(e.what());
        return 0.0;
    }
}

bool ExpressionParser::evaluateCondition(const QString& expression) {
    return evaluate(expression) != 0.0;
}

bool ExpressionParser::validate(const QString& expression) {
    evaluate(expression);
    return m_lastError.isEmpty();
}

// Recursive descent parser implementation

void ExpressionParser::skipWhitespace() {
    while (!isAtEnd() && (m_expression[m_pos].isSpace() || m_expression[m_pos] == '\t')) {
        m_pos++;
    }
}

bool ExpressionParser::match(char c) {
    skipWhitespace();
    if (!isAtEnd() && m_expression[m_pos] == c) {
        m_pos++;
        return true;
    }
    return false;
}

bool ExpressionParser::match(const QString& s) {
    skipWhitespace();
    if (m_pos + s.length() <= m_expression.length() && 
        m_expression.mid(m_pos, s.length()) == s) {
        m_pos += s.length();
        return true;
    }
    return false;
}

char ExpressionParser::peek() {
    skipWhitespace();
    return isAtEnd() ? '\0' : m_expression[m_pos].toLatin1();
}

char ExpressionParser::advance() {
    return isAtEnd() ? '\0' : m_expression[m_pos++].toLatin1();
}

bool ExpressionParser::isAtEnd() {
    return m_pos >= m_expression.length();
}

double ExpressionParser::parseExpression() {
    return parseTernary();
}

double ExpressionParser::parseTernary() {
    double condition = parseLogicalOr();
    
    skipWhitespace();
    if (match('?')) {
        double trueVal = parseExpression();
        if (!match(':')) {
            m_lastError = "Expected ':' in ternary expression";
            return 0;
        }
        double falseVal = parseExpression();
        return condition != 0 ? trueVal : falseVal;
    }
    
    return condition;
}

double ExpressionParser::parseLogicalOr() {
    double left = parseLogicalAnd();
    
    while (match("||")) {
        double right = parseLogicalAnd();
        left = (left != 0 || right != 0) ? 1.0 : 0.0;
    }
    
    return left;
}

double ExpressionParser::parseLogicalAnd() {
    double left = parseBitwiseOr();
    
    while (match("&&")) {
        double right = parseBitwiseOr();
        left = (left != 0 && right != 0) ? 1.0 : 0.0;
    }
    
    return left;
}

double ExpressionParser::parseBitwiseOr() {
    double left = parseBitwiseXor();
    
    skipWhitespace();
    while (!isAtEnd() && peek() == '|' && m_pos + 1 < m_expression.length() && 
           m_expression[m_pos + 1] != '|') {
        advance();
        double right = parseBitwiseXor();
        left = static_cast<double>(static_cast<int>(left) | static_cast<int>(right));
    }
    
    return left;
}

double ExpressionParser::parseBitwiseXor() {
    double left = parseBitwiseAnd();
    
    while (match('^')) {
        double right = parseBitwiseAnd();
        left = static_cast<double>(static_cast<int>(left) ^ static_cast<int>(right));
    }
    
    return left;
}

double ExpressionParser::parseBitwiseAnd() {
    double left = parseEquality();
    
    skipWhitespace();
    while (!isAtEnd() && peek() == '&' && m_pos + 1 < m_expression.length() && 
           m_expression[m_pos + 1] != '&') {
        advance();
        double right = parseEquality();
        left = static_cast<double>(static_cast<int>(left) & static_cast<int>(right));
    }
    
    return left;
}

double ExpressionParser::parseEquality() {
    double left = parseRelational();
    
    while (true) {
        if (match("==")) {
            double right = parseRelational();
            left = (left == right) ? 1.0 : 0.0;
        } else if (match("!=")) {
            double right = parseRelational();
            left = (left != right) ? 1.0 : 0.0;
        } else {
            break;
        }
    }
    
    return left;
}

double ExpressionParser::parseRelational() {
    double left = parseShift();
    
    while (true) {
        if (match("<=")) {
            double right = parseShift();
            left = (left <= right) ? 1.0 : 0.0;
        } else if (match(">=")) {
            double right = parseShift();
            left = (left >= right) ? 1.0 : 0.0;
        } else if (match('<')) {
            double right = parseShift();
            left = (left < right) ? 1.0 : 0.0;
        } else if (match('>')) {
            double right = parseShift();
            left = (left > right) ? 1.0 : 0.0;
        } else {
            break;
        }
    }
    
    return left;
}

double ExpressionParser::parseShift() {
    double left = parseAdditive();
    
    while (true) {
        if (match("<<")) {
            double right = parseAdditive();
            left = static_cast<double>(static_cast<int>(left) << static_cast<int>(right));
        } else if (match(">>")) {
            double right = parseAdditive();
            left = static_cast<double>(static_cast<int>(left) >> static_cast<int>(right));
        } else {
            break;
        }
    }
    
    return left;
}

double ExpressionParser::parseAdditive() {
    double left = parseMultiplicative();
    
    while (true) {
        if (match('+')) {
            left = left + parseMultiplicative();
        } else if (match('-')) {
            left = left - parseMultiplicative();
        } else {
            break;
        }
    }
    
    return left;
}

double ExpressionParser::parseMultiplicative() {
    double left = parseUnary();
    
    while (true) {
        if (match('*')) {
            left = left * parseUnary();
        } else if (match('/')) {
            double divisor = parseUnary();
            if (divisor == 0) {
                m_lastError = "Division by zero";
                return 0;
            }
            left = left / divisor;
        } else if (match('%')) {
            int divisor = static_cast<int>(parseUnary());
            if (divisor == 0) {
                m_lastError = "Modulo by zero";
                return 0;
            }
            left = static_cast<int>(left) % divisor;
        } else {
            break;
        }
    }
    
    return left;
}

double ExpressionParser::parseUnary() {
    if (match('-')) {
        return -parseUnary();
    }
    if (match('+')) {
        return parseUnary();
    }
    if (match('!')) {
        return parseUnary() == 0 ? 1.0 : 0.0;
    }
    if (match('~')) {
        return static_cast<double>(~static_cast<int>(parseUnary()));
    }
    
    return parsePrimary();
}

double ExpressionParser::parsePrimary() {
    skipWhitespace();
    
    // Parentheses
    if (match('(')) {
        double result = parseExpression();
        if (!match(')')) {
            m_lastError = "Expected ')'";
            return 0;
        }
        return result;
    }
    
    // Number
    if (!isAtEnd() && (m_expression[m_pos].isDigit() || m_expression[m_pos] == '.')) {
        return parseNumber();
    }
    
    // Identifier (variable or function)
    if (!isAtEnd() && (m_expression[m_pos].isLetter() || m_expression[m_pos] == '_' || m_expression[m_pos] == '$')) {
        QString ident = parseIdentifier();
        
        skipWhitespace();
        // Check if it's a function call
        if (match('(')) {
            return parseFunction(ident);
        }
        
        // It's a variable
        if (m_variableResolver) {
            return m_variableResolver(ident);
        } else {
            m_lastError = QString("Unknown variable: %1").arg(ident);
            return 0;
        }
    }
    
    m_lastError = QString("Unexpected character: %1").arg(peek());
    return 0;
}

double ExpressionParser::parseFunction(const QString& funcName) {
    QVector<double> args;
    
    skipWhitespace();
    if (!match(')')) {
        // Parse arguments
        do {
            args.append(parseExpression());
        } while (match(','));
        
        if (!match(')')) {
            m_lastError = QString("Expected ')' after function arguments");
            return 0;
        }
    }
    
    // Look up function
    if (m_functions.contains(funcName)) {
        return m_functions[funcName](args);
    }
    
    m_lastError = QString("Unknown function: %1").arg(funcName);
    return 0;
}

double ExpressionParser::parseNumber() {
    int start = m_pos;
    bool hasDecimal = false;
    bool hasExponent = false;
    
    // Handle hex numbers
    if (m_pos + 1 < m_expression.length() && m_expression[m_pos] == '0' && 
        (m_expression[m_pos + 1] == 'x' || m_expression[m_pos + 1] == 'X')) {
        m_pos += 2;
        while (!isAtEnd() && (m_expression[m_pos].isDigit() || 
               (m_expression[m_pos] >= 'a' && m_expression[m_pos] <= 'f') ||
               (m_expression[m_pos] >= 'A' && m_expression[m_pos] <= 'F'))) {
            m_pos++;
        }
        bool ok;
        return m_expression.mid(start, m_pos - start).toInt(&ok, 16);
    }
    
    while (!isAtEnd()) {
        QChar c = m_expression[m_pos];
        if (c.isDigit()) {
            m_pos++;
        } else if (c == '.' && !hasDecimal && !hasExponent) {
            hasDecimal = true;
            m_pos++;
        } else if ((c == 'e' || c == 'E') && !hasExponent) {
            hasExponent = true;
            m_pos++;
            if (!isAtEnd() && (m_expression[m_pos] == '+' || m_expression[m_pos] == '-')) {
                m_pos++;
            }
        } else {
            break;
        }
    }
    
    return m_expression.mid(start, m_pos - start).toDouble();
}

QString ExpressionParser::parseIdentifier() {
    int start = m_pos;
    
    // Handle $ prefix for variables
    if (!isAtEnd() && m_expression[m_pos] == '$') {
        m_pos++;
    }
    
    while (!isAtEnd() && (m_expression[m_pos].isLetterOrNumber() || m_expression[m_pos] == '_')) {
        m_pos++;
    }
    
    // Handle array indexing [n]
    if (!isAtEnd() && m_expression[m_pos] == '[') {
        while (!isAtEnd() && m_expression[m_pos] != ']') {
            m_pos++;
        }
        if (!isAtEnd()) {
            m_pos++; // consume ']'
        }
    }
    
    return m_expression.mid(start, m_pos - start);
}
