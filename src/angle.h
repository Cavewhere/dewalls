#ifndef DEWALLS_ANGLE_H
#define DEWALLS_ANGLE_H

#include <QString>
#include <QSharedPointer>
#include "dewallsexport.h"

namespace dewalls {

class DEWALLS_LIB_EXPORT Angle
{
public:
    enum Unit {
        Invalid = 0,
        Radians = 1,
        Degrees = 2,
        Gradians = 3,
        MilsNATO = 4,
        PercentGrade = 5
    };

    static QString Name;

    static long double toBase(long double quantity, Unit fromUnit);
    static long double fromBase(long double quantity, Unit toUnit);
    static long double convert(long double quantity, Unit fromUnit, Unit toUnit);
    static QString symbolFor(Unit unit);
};

} // namespace dewalls

#endif // DEWALLS_ANGLE_H
