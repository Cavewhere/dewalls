#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "vector.h"
#include "wallstypes.h"
#include "unitizedmath.h"

using namespace dewalls;
using Catch::Matchers::WithinAbs;

typedef UnitizedDouble<Length> ULength;
typedef UnitizedDouble<Angle> UAngle;

void testInstance(ULength instY, ULength targetY, ULength fromY, ULength toY, ULength horizDist, ULength inch, QList<TapingMethodMeasurement> tape) {
    WallsUnits units;
    units.setInch(inch);

    units.setIncd(ULength(1, Length::Inches));
    units.setIncs(ULength(1, Length::Inches));
    units.setIncv(UAngle(2, Angle::Degrees));
    units.setTape(tape);

    INFO( "instY: " << instY );
    INFO( "targetY: " << targetY );
    INFO( "fromY: " << fromY );
    INFO( "toY: " << toY );
    INFO( "horizDist: " << horizDist );
    INFO( "inch: " << inch );

    ULength tapeFromY = tape[0] == TapingMethodMeasurement::InstrumentHeight ? instY : fromY;
    ULength tapeToY   = tape[1] == TapingMethodMeasurement::TargetHeight     ? targetY : toY;

    Vector vector;
    vector.setDistance(usqrt(usq(horizDist) + usq(tapeToY - tapeFromY)) - units.incd());
    vector.setFrontAzimuth(UAngle(0, Angle::Degrees) - units.inca());
    if (horizDist.isNonzero() && targetY != instY) {
        vector.setFrontInclination(uatan((targetY - instY) / horizDist) - units.incv());
    }
    vector.setInstHeight(instY - fromY - units.incs());
    vector.setTargetHeight(targetY - toY - units.incs());
    vector.setUnits(units);

    ULength expectedDist = usqrt(usq(toY + inch - fromY) + usq(horizDist));
    UAngle  expectedInc  = uatan2((toY + inch - fromY), horizDist);

    ULength instHeightAboveTape = instY - tapeFromY;
    ULength targetHeightAboveTape = targetY - tapeToY;

    bool isDiveShot = tape[0] == TapingMethodMeasurement::Station &&
            tape[1] == TapingMethodMeasurement::Station &&
            (!vector.frontInclination().isValid() || vector.frontInclination().isZero());

    if (!isDiveShot && uabs(instHeightAboveTape - targetHeightAboveTape) > vector.distance()) {
        CHECK_THROWS( vector.applyHeightCorrections() );
    }
    else {
        vector.applyHeightCorrections();

        const double tolerance = 1e-5; // Define your tolerance
        CHECK_THAT((vector.distance() + units.incd()).get(Length::Meters), WithinAbs(expectedDist.get(Length::Meters), tolerance));
        CHECK_THAT((vector.frontInclination() + units.incv()).get(Angle::Degrees), WithinAbs(expectedInc.get(Angle::Degrees), tolerance));
    }
}

QList<TapingMethodMeasurement> IT({TapingMethodMeasurement::InstrumentHeight, TapingMethodMeasurement::TargetHeight});
QList<TapingMethodMeasurement> IS({TapingMethodMeasurement::InstrumentHeight, TapingMethodMeasurement::Station});
QList<TapingMethodMeasurement> ST({TapingMethodMeasurement::Station, TapingMethodMeasurement::TargetHeight});
QList<TapingMethodMeasurement> SS({TapingMethodMeasurement::Station, TapingMethodMeasurement::Station});

void testInstance(ULength instY, ULength targetY, ULength fromY, ULength toY, ULength horizDist, ULength inch) {
    if (horizDist.isNonzero()) {
        testInstance(instY, targetY, fromY, toY, horizDist, inch, IT);
        testInstance(instY, targetY, fromY, toY, horizDist, inch, IS);
        testInstance(instY, targetY, fromY, toY, horizDist, inch, ST);
    }
    testInstance(instY, targetY, fromY, toY, horizDist, inch, SS);
}

TEST_CASE( "applyHeightCorrections", "[dewalls]" ) {
    SECTION( "vertical dive shots without inch" ) {
        testInstance(ULength(0, Length::Meters), ULength(0, Length::Meters), ULength(-8, Length::Meters), ULength(-3, Length::Meters), ULength(0, Length::Meters), ULength(0, Length::Meters));
        testInstance(ULength(0, Length::Meters), ULength(0, Length::Meters), ULength(-3, Length::Meters), ULength(-8, Length::Meters), ULength(0, Length::Meters), ULength(0, Length::Meters));
    }
    SECTION( "vertical dive shots with inch" ) {
        testInstance(ULength(0, Length::Meters), ULength(0, Length::Meters), ULength(-8, Length::Meters), ULength(-3, Length::Meters), ULength(0, Length::Meters), ULength(2, Length::Meters));
        testInstance(ULength(0, Length::Meters), ULength(0, Length::Meters), ULength(-3, Length::Meters), ULength(-8, Length::Meters), ULength(0, Length::Meters), ULength(2, Length::Meters));
    }
    SECTION( "near-vertical dive shots without inch") {
        testInstance(ULength(0, Length::Meters), ULength(0, Length::Meters), ULength(-3, Length::Meters), ULength(-18, Length::Meters), ULength(0.5, Length::Meters), ULength(0, Length::Meters), SS);
        testInstance(ULength(0, Length::Meters), ULength(0, Length::Meters), ULength(-18, Length::Meters), ULength(-3, Length::Meters), ULength(0.5, Length::Meters), ULength(0, Length::Meters), SS);
    }
    SECTION( "near-vertical dive shots with inch") {
        testInstance(ULength(0, Length::Meters), ULength(0, Length::Meters), ULength(-3, Length::Meters), ULength(-18, Length::Meters), ULength(0.5, Length::Meters), ULength(2, Length::Meters), SS);
        testInstance(ULength(0, Length::Meters), ULength(0, Length::Meters), ULength(-18, Length::Meters), ULength(-3, Length::Meters), ULength(0.5, Length::Meters), ULength(2, Length::Meters), SS);
    }
    SECTION( "various shots" ) {
        testInstance(ULength(3, Length::Meters), ULength(8, Length::Meters), ULength(2, Length::Meters), ULength(4, Length::Meters), ULength(7, Length::Meters), ULength(2, Length::Meters));
        testInstance(ULength(3, Length::Meters), ULength(8, Length::Meters), ULength(2, Length::Meters), ULength(9, Length::Meters), ULength(7, Length::Meters), ULength(2, Length::Meters));
        testInstance(ULength(3, Length::Meters), ULength(8, Length::Meters), ULength(4, Length::Meters), ULength(7, Length::Meters), ULength(7, Length::Meters), ULength(2, Length::Meters));
        testInstance(ULength(3, Length::Meters), ULength(8, Length::Meters), ULength(0, Length::Meters), ULength(0, Length::Meters), ULength(7, Length::Meters), ULength(2, Length::Meters));
    }
    SECTION( "ridiculous shots" ) {
        testInstance(ULength(3, Length::Meters), ULength(8, Length::Meters), ULength(4, Length::Meters), ULength(68, Length::Meters), ULength(7, Length::Meters), ULength(2, Length::Meters));
        testInstance(ULength(3, Length::Meters), ULength(8, Length::Meters), ULength(68, Length::Meters), ULength(68, Length::Meters), ULength(7, Length::Meters), ULength(2, Length::Meters));
        testInstance(ULength(3, Length::Meters), ULength(58, Length::Meters), ULength(2, Length::Meters), ULength(-62, Length::Meters), ULength(7, Length::Meters), ULength(2, Length::Meters));
    }
}
