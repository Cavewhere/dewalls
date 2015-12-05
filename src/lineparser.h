#ifndef DEWALLS_LINEPARSER_H
#define DEWALLS_LINEPARSER_H

#include <climits>
#include "segment.h"
#include <QString>
#include <QChar>
#include <QSet>
#include <QHash>
#include <QPair>
#include <QList>
#include <QStringList>
#include "segmentparseexpectedexception.h"
#include <initializer_list>
#include <functional>

#include <iostream>

namespace dewalls {

class LineParser
{
public:
    LineParser();
    LineParser(Segment line);

    virtual void reset(QString newLine);
    virtual void reset(Segment newLine);

    bool isAtEnd() const;

    void addExpected(const SegmentParseExpectedException& ex);

    SegmentParseExpectedException allExpected();
    void throwAllExpected();
    void throwAllExpected(const SegmentParseExpectedException& finalEx);

    void expect(const QChar& c, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    void expect(const QString& c, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    Segment expect(const QRegExp& rx, std::initializer_list<QString> expectedItems);
    Segment expect(QRegExp& rx, std::initializer_list<QString> expectedItems);
    Segment expect(QRegExp &rx, QList<QString> expectedItems);

    template<typename F>
    QChar expectChar(F charPredicate, std::initializer_list<QString> expectedItems);

    Segment whitespace();
    Segment nonwhitespace();

    bool maybeWhitespace();

    static const QRegExp unsignedIntLiteralRx;
    uint unsignedIntLiteral();

    static const QHash<QChar, int> intSignSignums;
    int intLiteral();

    static const QRegExp unsignedDoubleLiteralRx;
    double unsignedDoubleLiteral();

    static const QHash<QChar, double> signSignums;
    double doubleLiteral();

    template<typename V>
    V oneOfMap(QHash<QChar, V> map);

    template<typename V>
    V oneOfMap(QHash<QChar, V> map, V elseValue);

    template<typename V>
    V oneOfMapLowercase(const QRegExp& rx, QHash<QString, V> map);

    template<typename V>
    V oneOfMapLowercase(QRegExp& rx, QHash<QString, V> map);

    template<typename F>
    void throwAllExpected(F production);

    template<typename F>
    void oneOf(F production);

    template<typename F, typename... Args>
    void oneOf(F production, Args... args);

    template<typename R, typename F>
    void oneOfR(R& result, F production);

    template<typename R, typename F, typename... Args>
    void oneOfR(R& result, F production, Args... args);

    template<typename F>
    void oneOfWithLookahead(F production);

    template<typename F, typename... Args>
    void oneOfWithLookahead(F production, Args... args);

    template<typename F>
    bool maybe(F production);

    template<typename R, typename F>
    bool maybe(R& result, F production);

    bool maybeChar(QChar c);

    void endOfLine();

    Segment remaining();

    static const QRegExp whitespaceRx;
    static const QRegExp nonwhitespaceRx;

protected:
    Segment _line;
    int _i;
    int _expectedIndex;
    QStringList _expectedItems;
};

template<typename F>
void LineParser::throwAllExpected(F production)
{
    try
    {
        production();
    }
    catch (const SegmentParseExpectedException& ex)
    {
        throwAllExpected(ex);
    }
}

template<typename F>
void LineParser::oneOf(F production)
{
    try
    {
        production();
    }
    catch (const SegmentParseExpectedException& ex)
    {
        throwAllExpected(ex);
    }
}

template<typename F, typename... Args>
void LineParser::oneOf(F production, Args... args)
{
    int start = _i;
    try
    {
        production();
    }
    catch (const SegmentParseExpectedException& ex)
    {
        if (_i > start)
        {
            throwAllExpected(ex);
        }
        addExpected(ex);
        oneOf(args...);
    }
}

template<typename R, typename F>
void LineParser::oneOfR(R& result, F production)
{
    try
    {
        result = production();
    }
    catch (const SegmentParseExpectedException& ex)
    {
        throwAllExpected(ex);
    }
}

template<typename R, typename F, typename... Args>
void LineParser::oneOfR(R& result, F production, Args... args)
{
    int start = _i;
    try
    {
        result = production();
    }
    catch (const SegmentParseExpectedException& ex)
    {
        if (_i > start)
        {
            throwAllExpected(ex);
        }
        addExpected(ex);
        oneOfR(result, args...);
    }
}

//template<typename R, class O>
//void oneOfOwnR(R& result, R (O::*production)())
//{
//    try
//    {
//        return (this->*production)();
//    }
//    catch (const SegmentParseExpectedException& ex)
//    {
//        throwAllExpected(ex);
//    }
//}

//template<typename R, class O, typename... Args>
//void oneOfOwnR(R& result, R (O::*production)(), Args... args)
//{
//    int start = _i;
//    try
//    {
//        return (this->*production)();
//    }
//    catch (const SegmentParseExpectedException& ex)
//    {
//        if (_i > start)
//        {
//            throwAllExpected(ex);
//        }
//        addExpected(ex);
//        oneOfOwnR(result, args...);
//    }
//}

template<typename F>
void LineParser::oneOfWithLookahead(F production)
{
    try
    {
        production();
    }
    catch (const SegmentParseExpectedException& ex)
    {
//        std::cout << ex.message().toStdString() << std::endl;
        throwAllExpected(ex);
    }
}

template<typename F, typename... Args>
void LineParser::oneOfWithLookahead(F production, Args... args)
{
    int start = _i;
    try
    {
        production();
    }
    catch (const SegmentParseExpectedException& ex)
    {
//        std::cout << ex.message().toStdString() << std::endl;
        addExpected(ex);
        _i = start;
        oneOfWithLookahead(args...);
    }
}

template<typename V>
V LineParser::oneOfMap(QHash<QChar, V> map)
{
    QChar c;
    if (_i >= _line.length() || !map.contains(c = _line.at(_i)))
    {
        QList<QString> expectedItems;
        for (QChar exp : map.keys())
        {
            expectedItems << QString(exp);
        }
        throw SegmentParseExpectedException(_line.atAsSegment(_i), expectedItems);
    }
    _i++;
    return map[c];
}

template<typename V>
V LineParser::oneOfMap(QHash<QChar, V> map, V elseValue)
{
   try
    {
        return oneOfMap(map);
    }
    catch (const SegmentParseExpectedException& ex)
    {
        addExpected(ex);
        return elseValue;
    }
}

template<typename V>
V LineParser::oneOfMapLowercase(const QRegExp& rx, QHash<QString, V> map)
{
    QRegExp rxCopy = rx;
    return oneOfMapLowercase(rxCopy, map);
}

template<typename V>
V LineParser::oneOfMapLowercase(QRegExp& rx, QHash<QString, V> map)
{
    int start = _i;
    Segment seg = expect(rx, map.keys());
    QString str = seg.toLower();
    if (!map.contains(str))
    {
        _i = start;
        throw SegmentParseExpectedException(seg, map.keys());
    }
    return map[str];
}

template<typename F>
bool LineParser::maybe(F production)
{
    int start = _i;
    try
    {
        production();
        return true;
    }
    catch (const SegmentParseExpectedException& ex)
    {
        if (_i > start)
        {
            throwAllExpected(ex);
        }
        addExpected(ex);
        return false;
    }
}

template<typename R, typename F>
bool LineParser::maybe(R& result, F production)
{
    int start = _i;
    try
    {
        result = production();
        return true;
    }
    catch (const SegmentParseExpectedException& ex)
    {
        if (_i > start)
        {
            throwAllExpected(ex);
        }
        addExpected(ex);
        return false;
    }
}

template<typename F>
QChar LineParser::expectChar(F charPredicate, std::initializer_list<QString> expectedItems)
{
    QChar c;
    if (_i >= _line.length() || !charPredicate(c = _line.at(_i)))
    {
        throw SegmentParseExpectedException(_line.atAsSegment(_i), expectedItems);
    }
    _i++;
    return c;
}

} // namespace dewalls

#endif // DEWALLS_LINEPARSER_H
