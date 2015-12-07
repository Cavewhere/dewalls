#include "../lib/catch.hpp"
#include "../src/wallsparser.h"
#include "approx.h"
#include "unitizedmath.h"

#include <iostream>

using namespace dewalls;

typedef UnitizedDouble<Length> ULength;
typedef UnitizedDouble<Angle> UAngle;

TEST_CASE( "general tests", "[dewalls]" ) {
    WallsParser parser;

    REQUIRE( parser.units().vectorType() == VectorType::CT );
    REQUIRE( parser.units().dUnit() == Length::meters() );
    REQUIRE( parser.units().sUnit() == Length::meters() );
    REQUIRE( parser.units().aUnit() == Angle::degrees() );
    REQUIRE( parser.units().abUnit() == Angle::degrees() );
    REQUIRE( parser.units().vUnit() == Angle::degrees() );
    REQUIRE( parser.units().vbUnit() == Angle::degrees() );
    REQUIRE( parser.units().lrud() == LrudType::From );
    REQUIRE( parser.units().prefix().isEmpty() );
    REQUIRE( parser.units().typeabCorrected() == false );
    REQUIRE( parser.units().typevbCorrected() == false );
    REQUIRE( parser.units().rect().isZero() );
    REQUIRE( parser.units().decl().isZero() );
    REQUIRE( parser.units().grid().isZero() );
    REQUIRE( parser.units().incd().isZero() );
    REQUIRE( parser.units().incs().isZero() );
    REQUIRE( parser.units().inca().isZero() );
    REQUIRE( parser.units().incab().isZero() );
    REQUIRE( parser.units().incv().isZero() );
    REQUIRE( parser.units().incvb().isZero() );
    REQUIRE( parser.units().inch().isZero() );
    REQUIRE( parser.date().isNull() );
    REQUIRE( parser.units().case_() == CaseType::Mixed );
    REQUIRE( parser.units().flag().isNull() );
    REQUIRE( parser.segment().isNull() );
    REQUIRE( parser.units().uvh() == 1.0 );
    REQUIRE( parser.units().uvv() == 1.0 );

    SECTION( "vector line parsing tests" ) {
        Vector vector;
        QList<WallsMessage> messages;

        QObject::connect(&parser, &WallsParser::parsedVector, [&](Vector v) { vector = v; });
        QObject::connect(&parser, &WallsParser::message, [&](WallsMessage m) { messages << m; });

        parser.parseLine("A1 A2 2.5 350 2.3");

        REQUIRE( vector.from() == "A1" );
        REQUIRE( vector.to() == "A2" );
        REQUIRE( vector.distance() == ULength(2.5, Length::meters()) );
        REQUIRE( vector.frontAzimuth() == UAngle(350, Angle::degrees()) );
        REQUIRE( !vector.backAzimuth().isValid() );
        REQUIRE( vector.frontInclination() == UAngle(2.3, Angle::degrees()) );
        REQUIRE( !vector.backInclination().isValid() );

        SECTION( "backsights" ) {
            parser.parseLine("A1 A2 2.5 350/349 2.3/2.4");

            REQUIRE( vector.from() == "A1" );
            REQUIRE( vector.to() == "A2" );
            REQUIRE( vector.distance() == ULength(2.5, Length::meters()) );
            REQUIRE( vector.frontAzimuth() == UAngle(350, Angle::degrees()) );
            REQUIRE( vector.backAzimuth() == UAngle(349, Angle::degrees()) );
            REQUIRE( vector.frontInclination() == UAngle(2.3, Angle::degrees()) );
            REQUIRE( vector.backInclination() == UAngle(2.4, Angle::degrees()) );
         }

        SECTION( "frontsights/backsights can be omitted" ) {
            parser.parseLine("A1 A2 2.5 350/-- --/2.4");

            REQUIRE( vector.frontAzimuth() == UAngle(350, Angle::degrees()) );
            REQUIRE( !vector.backAzimuth().isValid() );
            REQUIRE( !vector.frontInclination().isValid() );
            REQUIRE( vector.backInclination() == UAngle(2.4, Angle::degrees()) );
        }

        SECTION( "distance" ) {
            SECTION( "distance can't be omitted" ) {
                CHECK_THROWS( parser.parseLine("A B -- 350 2.5") );
            }

            SECTION( "can't be negative" ) {
                CHECK_THROWS( parser.parseLine("A B -0.001 350 4") );
            }

            SECTION( "dUnit affects distance" ) {
                parser.parseLine("#units d=feet");
                parser.parseLine("A B 2.5 350 4");
                REQUIRE( vector.distance() == ULength(2.5, Length::feet()) );
            }

            SECTION( "sUnit doesn't affect distance" ) {
                parser.parseLine("#units s=feet");
                parser.parseLine("A B 2.5 350 4");
                REQUIRE( vector.distance() == ULength(2.5, Length::meters()) );
            }
        }

        SECTION( "incd" ) {
            parser.parseLine("#units incd=-2");
            SECTION( "error when incd changes measurement sign" ) {
                // zero-length shots should not be affected
                parser.parseLine("A B 0 350 4");
                // shots long enough should be OK
                parser.parseLine("A B 2.1 350 4");
                CHECK_THROWS( parser.parseLine("A B 1 350 4") );
            }
        }

        SECTION( "azimuth" ) {
            SECTION( "azimuth can be omitted for vertical shots" ) {
                parser.parseLine("A B 2.5 -- 90");
                parser.parseLine("A B 2.5 -- 90/-90");
                parser.parseLine("A B 2.5 -- --/90");
                parser.parseLine("A B 2.5 -- --/-90");
                parser.parseLine("A B 2.5 -- 90/-90");
                parser.parseLine("A B 2.5 --/-- 90");
                parser.parseLine("A B 2.5 -- -90");
                parser.parseLine("A B 2.5 -- 100g");
                parser.parseLine("A B 2.5 -- -100g");
            }

            SECTION( "frontsight azimuth can be omitted without dashes" ) {
                parser.parseLine("A B 2.5 /235 0");
            }

            SECTION( "azimuth can't be omitted for non-vertical shots" ) {
                CHECK_THROWS( parser.parseLine("A B 2.5 -- 45") );
                CHECK_THROWS( parser.parseLine("A B 2.5 --/-- 45") );
            }

            SECTION( "aUnit" ) {
                parser.parseLine("#units a=grads");
                parser.parseLine("A B 1 2/3 4");
                REQUIRE( vector.frontAzimuth() == UAngle(2, Angle::gradians()) );
                REQUIRE( vector.backAzimuth() == UAngle(3, Angle::degrees()) );
            }

            SECTION( "abUnit" ) {
                parser.parseLine("#units ab=grads");
                parser.parseLine("A B 1 2/3 4");
                REQUIRE( vector.frontAzimuth() == UAngle(2, Angle::degrees()) );
                REQUIRE( vector.backAzimuth() == UAngle(3, Angle::gradians()) );
            }

            SECTION( "a/ab unit" ) {
                parser.parseLine("#units a/ab=grads");
                parser.parseLine("A B 1 2/3 4");
                REQUIRE( vector.frontAzimuth() == UAngle(2, Angle::gradians()) );
                REQUIRE( vector.backAzimuth() == UAngle(3, Angle::gradians()) );
            }

            SECTION( "parser warns if fs/bs difference exceeds tolerance" ) {
                messages.clear();
                parser.parseLine("A B 1 1/179 4");
                REQUIRE( messages.size() == 0 );

                messages.clear();
                parser.parseLine("A B 1 1/183 4");
                REQUIRE( messages.size() == 0 );

                messages.clear();
                parser.parseLine("A B 1 1/184 4");
                REQUIRE( messages.size() == 1 );
                REQUIRE( messages[0].message().contains("exceeds") );

                messages.clear();
                parser.parseLine("#units typeab=c");
                parser.parseLine("A B 1 1/3 4");
                REQUIRE( messages.size() == 0 );

                messages.clear();
                parser.parseLine("A B 1 1/359 4");
                REQUIRE( messages.size() == 0 );

                messages.clear();
                parser.parseLine("A B 1 359/1 4");
                REQUIRE( messages.size() == 0 );

                messages.clear();
                parser.parseLine("#units typeab=c,5");
                parser.parseLine("A B 1 1/6 4");
                REQUIRE( messages.size() == 0 );

                messages.clear();
                parser.parseLine("A B 1 1/7 4");
                REQUIRE( messages.size() == 1 );
            }
        }

        SECTION( "inclination" ) {
            SECTION( "vUnit" ) {
                parser.parseLine("#units v=grads");
                parser.parseLine("A B 1 2 3/4");
                REQUIRE( vector.frontInclination() == UAngle(3, Angle::gradians()) );
                REQUIRE( vector.backInclination() == UAngle(4, Angle::degrees()) );
            }

            SECTION( "backsight inclination can be omitted without dashes" ) {
                parser.parseLine("A B 1 2 /3");
            }

            SECTION( "vbUnit" ) {
                parser.parseLine("#units vb=grads");
                parser.parseLine("A B 1 2 3/4");
                REQUIRE( vector.frontInclination() == UAngle(3, Angle::degrees()) );
                REQUIRE( vector.backInclination() == UAngle(4, Angle::gradians()) );
            }

            SECTION( "v/vb unit" ) {
                parser.parseLine("#units v/vb=grads");
                parser.parseLine("A B 1 2 3/4");
                REQUIRE( vector.frontInclination() == UAngle(3, Angle::gradians()) );
                REQUIRE( vector.backInclination() == UAngle(4, Angle::gradians()) );
            }

            SECTION( "parser warns if fs/bs difference exceeds tolerance" ) {
                messages.clear();
                parser.parseLine("A B 1 2 4/-6");
                REQUIRE( messages.size() == 0 );

                messages.clear();
                parser.parseLine("A B 1 2 4/-2");
                REQUIRE( messages.size() == 0 );

                messages.clear();
                parser.parseLine("A B 1 2 4/-7");
                REQUIRE( messages.size() == 1 );
                REQUIRE( messages[0].message().contains("exceeds") );

                messages.clear();
                parser.parseLine("#units typevb=c");
                parser.parseLine("A B 1 2 1/3");
                REQUIRE( messages.size() == 0 );

                messages.clear();
                parser.parseLine("A B 1 2 1/-1");
                REQUIRE( messages.size() == 0 );

                messages.clear();
                parser.parseLine("#units typevb=c,5");
                parser.parseLine("A B 1 2 1/6");
                REQUIRE( messages.size() == 0 );

                messages.clear();
                parser.parseLine("A B 1 2 1/7");
                REQUIRE( messages.size() == 1 );
            }
        }

        SECTION( "rect data lines" ) {
            parser.parseLine("#units rect");
            parser.parseLine("A B 1 2 3");
            REQUIRE( vector.east() == ULength(1, Length::meters()) );
            REQUIRE( vector.north() == ULength(2, Length::meters()) );
            REQUIRE( vector.rectUp() == ULength(3, Length::meters()) );

            REQUIRE_THROWS( parser.parseLine("A B 1 2") );

            SECTION( "measurements can be reordered" ) {
                parser.parseLine("#units order=nue");
                parser.parseLine("A B 1 2 3");
                REQUIRE( vector.north() == ULength(1, Length::meters()) );
                REQUIRE( vector.rectUp() == ULength(2, Length::meters()) );
                REQUIRE( vector.east() == ULength(3, Length::meters()) );
            }

            SECTION( "up can be omitted" ) {
                parser.parseLine("#units order=ne");
                parser.parseLine("A B 1 2");
                REQUIRE( vector.north() == ULength(1, Length::meters()) );
                REQUIRE( vector.east() == ULength(2, Length::meters()) );
            }
        }

        SECTION( "splay shots" ) {
            parser.parseLine("A - 2.5 350 5");
            parser.parseLine("- B 2.5 350 5");
            REQUIRE_THROWS( parser.parseLine("- - 2.5 350 5") );
        }

        SECTION( "compass/tape measurements can be reordered" ) {
            parser.parseLine("#units order=avd");
            parser.parseLine("A B 1 2 3");
            REQUIRE( vector.frontAzimuth() == UAngle(1, Angle::degrees()) );
            REQUIRE( vector.frontInclination() == UAngle(2, Angle::degrees()) );
            REQUIRE( vector.distance() == ULength(3, Length::meters()) );
        }

        SECTION( "inclination can be omitted from order" ) {
            parser.parseLine("#units order=da");
            parser.parseLine("A B 1 2");
            REQUIRE( vector.distance() == ULength(1, Length::meters()) );
            REQUIRE( vector.frontAzimuth() == UAngle(2, Angle::degrees()) );
            REQUIRE( !vector.frontInclination().isValid() );
        }

        SECTION( "instrument and target heights" ) {
            parser.parseLine("A B 1 2 3 4 5");
            REQUIRE( vector.instHeight() == ULength(4, Length::meters()) );
            REQUIRE( vector.targetHeight() == ULength(5, Length::meters()) );

            SECTION( "are affected by sUnit" ) {
                parser.parseLine("#units s=feet");
                parser.parseLine("A B 1 2 3 4 5");
                REQUIRE( vector.instHeight() == ULength(4, Length::feet()) );
                REQUIRE( vector.targetHeight() == ULength(5, Length::feet()) );
            }

            SECTION( "are not affected by dUnit" ) {
                parser.parseLine("#units d=feet");
                parser.parseLine("A B 1 2 3 4 5");
                REQUIRE( vector.instHeight() == ULength(4, Length::meters()) );
                REQUIRE( vector.targetHeight() == ULength(5, Length::meters()) );
            }

            SECTION( "target only" ) {
                parser.parseLine("#units tape=st");
                parser.parseLine("A B 1 2 3 4");
                REQUIRE( !vector.instHeight().isValid() );
                REQUIRE( vector.targetHeight() == ULength(4, Length::meters()) );
                REQUIRE_THROWS( parser.parseLine("A B 1 2 3 4 5") );
            }

            SECTION( "with inclination omitted" ) {
                parser.parseLine("A B 1 2 -- 4 5");
                REQUIRE( vector.instHeight() == ULength(4, Length::meters()) );
                REQUIRE( vector.targetHeight() == ULength(5, Length::meters()) );
            }

            SECTION( "disabled" ) {
                parser.parseLine("#units tape=ss");
                REQUIRE_THROWS( parser.parseLine("A B 1 2 3 4") );
            }

            SECTION( "aren't allowed for rect data lines" ) {
                parser.parseLine("#units rect");
                REQUIRE_THROWS( parser.parseLine("A B 1 2 3 4 5") );
            }
        }

        SECTION( "variance overrides" ) {
            SECTION( "both can't be omitted" )  {
               REQUIRE_THROWS( parser.parseLine("A B 1 2 3 (,)") );
            }

            SECTION( "one can be omitted" ) {
                parser.parseLine("A B 1 2 3 (?,)");
                CHECK( vector.horizVariance() == VarianceOverride::FLOATED );
                CHECK( vector.vertVariance().isNull() );

                parser.parseLine("A B 1 2 3 (,?)");
                CHECK( vector.horizVariance().isNull() );
                CHECK( vector.vertVariance() == VarianceOverride::FLOATED );
            }

            SECTION( "various types" ) {
                parser.parseLine("A B 1 2 3 (?,*)");
                CHECK( vector.horizVariance() == VarianceOverride::FLOATED );
                CHECK( vector.vertVariance() == VarianceOverride::FLOATED_TRAVERSE );

                parser.parseLine("A B 1 2 3 (1000f,r4.5f)");
                REQUIRE( vector.horizVariance()->type() == VarianceOverride::Type::LENGTH_OVERRIDE );
                CHECK( vector.horizVariance().staticCast<LengthOverride>()->lengthOverride() == ULength(1000, Length::feet()) );
                REQUIRE( vector.vertVariance()->type() == VarianceOverride::Type::RMS_ERROR );
                CHECK( vector.vertVariance().staticCast<RMSError>()->error() == ULength(4.5, Length::feet()) );
            }
        }

        SECTION( "lruds" ) {
            parser.parseLine("A B 1 2 3 *4,5,6,7*");
            REQUIRE( vector.left() == ULength(4, Length::meters()) );
            REQUIRE( vector.right() == ULength(5, Length::meters()) );
            REQUIRE( vector.up() == ULength(6, Length::meters()) );
            REQUIRE( vector.down() == ULength(7, Length::meters()) );

            parser.parseLine("A B 1 2 3 *4 5 6 7*");
            REQUIRE( vector.left() == ULength(4, Length::meters()) );
            REQUIRE( vector.right() == ULength(5, Length::meters()) );
            REQUIRE( vector.up() == ULength(6, Length::meters()) );
            REQUIRE( vector.down() == ULength(7, Length::meters()) );

            parser.parseLine("A B 1 2 3 <4 5 6 7>");
            REQUIRE( vector.left() == ULength(4, Length::meters()) );
            REQUIRE( vector.right() == ULength(5, Length::meters()) );
            REQUIRE( vector.up() == ULength(6, Length::meters()) );
            REQUIRE( vector.down() == ULength(7, Length::meters()) );

            parser.parseLine("A B 1 2 3 <4,5,6,7>");
            REQUIRE( vector.left() == ULength(4, Length::meters()) );
            REQUIRE( vector.right() == ULength(5, Length::meters()) );
            REQUIRE( vector.up() == ULength(6, Length::meters()) );
            REQUIRE( vector.down() == ULength(7, Length::meters()) );

            SECTION( "can omit lruds" ) {
                parser.parseLine("A B 1 2 3 <4,--,6,-->");
                REQUIRE( vector.left() == ULength(4, Length::meters()) );
                REQUIRE( !vector.right().isValid() );
                REQUIRE( vector.up() == ULength(6, Length::meters()) );
                REQUIRE( !vector.down().isValid() );
            }

            SECTION( "negative numbers not allowed" ) {
                CHECK_THROWS( parser.parseLine("A B 1 2 3 *-4,5,6,7*") );
            }

            SECTION( "can unitize individual lruds" ) {
                parser.parseLine("A B 1 2 3 *4f,5m,6i3,i7*");
                REQUIRE( vector.left() == ULength(4, Length::feet()) );
                REQUIRE( vector.right() == ULength(5, Length::meters()) );
                REQUIRE( vector.up() == ULength(6 * 12 + 3, Length::inches()) );
                REQUIRE( vector.down() == ULength(7, Length::inches()) );
            }

            SECTION( "sUnit affects lruds" ) {
                parser.parseLine("#units s=feet");
                parser.parseLine("A B 1 2 3 *4,5,6,7*");
                REQUIRE( vector.left() == ULength(4, Length::feet()) );
                REQUIRE( vector.right() == ULength(5, Length::feet()) );
                REQUIRE( vector.up() == ULength(6, Length::feet()) );
                REQUIRE( vector.down() == ULength(7, Length::feet()) );
            }

            SECTION( "dUnit doesn't affect lruds" ) {
                parser.parseLine("#units d=feet");
                parser.parseLine("A B 1 2 3 *4,5,6,7*");
                REQUIRE( vector.left() == ULength(4, Length::meters()) );
                REQUIRE( vector.right() == ULength(5, Length::meters()) );
                REQUIRE( vector.up() == ULength(6, Length::meters()) );
                REQUIRE( vector.down() == ULength(7, Length::meters()) );
            }

            SECTION( "malformed lruds" ) {
                CHECK_THROWS( parser.parseLine("A B 1 2 3 *4,5,6,7>") );
                CHECK_THROWS( parser.parseLine("A B 1 2 3 <4,5,6,7*") );
                CHECK_THROWS( parser.parseLine("A B 1 2 3 *4,5 6,7*") );
                CHECK_THROWS( parser.parseLine("A B 1 2 3 <4,5 6,7>") );
                CHECK_THROWS( parser.parseLine("A B 1 2 3 <4,5,6,>") );
                CHECK_THROWS( parser.parseLine("A B 1 2 3 <4,5,6>") );
                CHECK_THROWS( parser.parseLine("A B 1 2 3 <>") );
                CHECK_THROWS( parser.parseLine("A B 1 2 3 < >") );
                CHECK_THROWS( parser.parseLine("A B 1 2 3 <,>") );
            }

            SECTION( "can change lrud order" ) {
                parser.parseLine("#units lrud=from:urld");
                parser.parseLine("A B 1 2 3 *4,5,6,7*");
                REQUIRE( vector.up() == ULength(4, Length::meters()) );
                REQUIRE( vector.right() == ULength(5, Length::meters()) );
                REQUIRE( vector.left() == ULength(6, Length::meters()) );
                REQUIRE( vector.down() == ULength(7, Length::meters()) );
            }
        }

        SECTION( "lrud/station name ambiguity" ) {
            parser.parseLine("A *1 2 3 4");
            CHECK( vector.to() == "*1" );
            CHECK( vector.distance() == ULength(2, Length::meters()) );
            CHECK( vector.frontAzimuth() == UAngle(3, Angle::degrees()) );
            CHECK( vector.frontInclination() == UAngle(4, Angle::degrees()) );
            CHECK( vector.left() == ULength() );
            CHECK( vector.right() == ULength() );
            CHECK( vector.up() == ULength() );
            CHECK( vector.down() == ULength() );

            parser.parseLine("A <1 2 3 4");
            CHECK( vector.to() == "<1" );
            CHECK( vector.distance() == ULength(2, Length::meters()) );
            CHECK( vector.frontAzimuth() == UAngle(3, Angle::degrees()) );
            CHECK( vector.frontInclination() == UAngle(4, Angle::degrees()) );
            CHECK( vector.left() == ULength() );
            CHECK( vector.right() == ULength() );
            CHECK( vector.up() == ULength() );
            CHECK( vector.down() == ULength() );

            parser.parseLine("A *1 2 3 4 *5,6,7,8*");
            CHECK( vector.to() == "*1" );
            CHECK( vector.distance() == ULength(2, Length::meters()) );
            CHECK( vector.frontAzimuth() == UAngle(3, Angle::degrees()) );
            CHECK( vector.frontInclination() == UAngle(4, Angle::degrees()) );
            CHECK( vector.left() == ULength(5, Length::meters()) );
            CHECK( vector.right() == ULength(6, Length::meters()) );
            CHECK( vector.up() == ULength(7, Length::meters()) );
            CHECK( vector.down() == ULength(8, Length::meters()) );

            parser.parseLine("A *1 2 3 4 *");
            CHECK( vector.to() == QString() );
            CHECK( vector.distance() == ULength() );
            CHECK( vector.frontAzimuth() == UAngle() );
            CHECK( vector.frontInclination() == UAngle() );
            CHECK( vector.left() == ULength(1, Length::meters()) );
            CHECK( vector.right() == ULength(2, Length::meters()) );
            CHECK( vector.up() == ULength(3, Length::meters()) );
            CHECK( vector.down() == ULength(4, Length::meters()) );

            parser.parseLine("A <1 2 3 4 <5,6,7,8>");
            CHECK( vector.to() == "<1" );
            CHECK( vector.distance() == ULength(2, Length::meters()) );
            CHECK( vector.frontAzimuth() == UAngle(3, Angle::degrees()) );
            CHECK( vector.frontInclination() == UAngle(4, Angle::degrees()) );
            CHECK( vector.left() == ULength(5, Length::meters()) );
            CHECK( vector.right() == ULength(6, Length::meters()) );
            CHECK( vector.up() == ULength(7, Length::meters()) );
            CHECK( vector.down() == ULength(8, Length::meters()) );

            parser.parseLine("A <1 2 3 4 >");
            CHECK( vector.to() == QString() );
            CHECK( vector.distance() == ULength() );
            CHECK( vector.frontAzimuth() == UAngle() );
            CHECK( vector.frontInclination() == UAngle() );
            CHECK( vector.left() == ULength(1, Length::meters()) );
            CHECK( vector.right() == ULength(2, Length::meters()) );
            CHECK( vector.up() == ULength(3, Length::meters()) );
            CHECK( vector.down() == ULength(4, Length::meters()) );
        }

        SECTION( "valid spacing" ) {
            parser.parseLine("   A B 1 2 3(?,?)*4,5,6,7*#s blah;test");
        }
    }

    SECTION( "prefixes" ) {
        parser.parseLine("#units prefix=a");
        CHECK( parser.units().processStationName("b") == "a:b" );
        CHECK( parser.units().processStationName("d:b") == "d:b" );
        CHECK( parser.units().processStationName(":b") == "b" );

        parser.parseLine("#units prefix2=c");
        CHECK( parser.units().processStationName("b") == "c:a:b" );
        CHECK( parser.units().processStationName(":b") == "c::b" );
        CHECK( parser.units().processStationName("d:b") == "c:d:b" );
        CHECK( parser.units().processStationName("::b") == "b" );
        CHECK( parser.units().processStationName(":::::b") == "b" );

        parser.parseLine("#units prefix1");
        CHECK( parser.units().processStationName("b") == "c::b" );
    }

    SECTION( "fixed stations" ) {
        FixStation station;

        QObject::connect(&parser, &WallsParser::parsedFixStation, [&](FixStation s) { station = s; });

        parser.parseLine("#FIX A1 W97:43:52.5 N31:16:45 323f (?,*) /Entrance #s blah ;dms with ft elevations");
        REQUIRE( station.name() == "A1" );
        REQUIRE( station.longitude() == UAngle(-97 - (43 + 52.5 / 60.0) / 60.0, Angle::degrees()) );
        REQUIRE( station.latitude() == UAngle(31 + (16 + 45 / 60.0) / 60.0, Angle::degrees()) );
        REQUIRE( station.rectUp() == ULength(323, Length::feet()) );
        REQUIRE( station.horizVariance() == VarianceOverride::FLOATED );
        REQUIRE( station.vertVariance() == VarianceOverride::FLOATED_TRAVERSE );
        REQUIRE( station.note() == "Entrance" );
        REQUIRE( station.segment() == "blah" );
        REQUIRE( station.comment() == "dms with ft elevations");

        parser.parseLine("#FIX A4 620775.38 3461050.67 98.45");
        REQUIRE( station.name() == "A4" );
        REQUIRE( station.east() == ULength(620775.38, Length::meters()) );
        REQUIRE( station.north() == ULength(3461050.67, Length::meters()) );
        REQUIRE( station.rectUp() == ULength(98.45, Length::meters()) );

        SECTION( "measurements can be reordered" ) {
            parser.parseLine("#units order=nue");
            parser.parseLine("#fix a 1 2 3");
            REQUIRE( station.north() == ULength(1, Length::meters()) );
            REQUIRE( station.rectUp() == ULength(2, Length::meters()) );
            REQUIRE( station.east() == ULength(3, Length::meters()) );
        }

        SECTION( "affected by dUnit" ) {
            parser.parseLine("#units d=feet");
            parser.parseLine("#fix a 1 2 3");
            REQUIRE( station.east() == ULength(1, Length::feet()) );
            REQUIRE( station.north() == ULength(2, Length::feet()) );
            REQUIRE( station.rectUp() == ULength(3, Length::feet()) );
        }

        SECTION( "valid spacing" ) {
            parser.parseLine("#FIX A1 W97:43:52.5 N31:16:45 323f(?,*)/Entrance#s blah;dms with ft elevations");
        }
    }

    SECTION( "save, restore, and reset" ) {
        parser.parseLine("#units lrud=from:urld");
        parser.parseLine("#units save");
        parser.parseLine("#units lrud=from:rldu");
        parser.parseLine("#units save");
        parser.parseLine("#units reset");
        REQUIRE( parser.units().lrudOrderString() == "LRUD" );
        parser.parseLine("#units restore");
        REQUIRE( parser.units().lrudOrderString() == "RLDU" );
        parser.parseLine("#units restore");
        REQUIRE( parser.units().lrudOrderString() == "URLD" );
    }

    SECTION( "can't do more than 10 saves" ) {
        for (int i = 0; i < 10; i++) {
            parser.parseLine("#units save");
        }
        REQUIRE_THROWS( parser.parseLine("#units save") );
    }

    SECTION( "can't restore more than number of times saved" ) {
        for (int i = 0; i < 4; i++) {
            parser.parseLine("#units save");
        }
        for (int i = 0; i < 4; i++) {
            parser.parseLine("#units restore");
        }
        REQUIRE_THROWS( parser.parseLine("#units restore") );
    }

    SECTION( "Walls' crazy macros" ) {
        parser.parseLine("#units $hello=\"der=vad pre\" $world=\"fix1=hello feet\"");
        REQUIRE( parser.macros()["hello"] == "der=vad pre" );
        REQUIRE( parser.macros()["world"] == "fix1=hello feet" );

        parser.parseLine("#units or$(hello)$(world)");

        REQUIRE( parser.units().ctOrder().size() == 3 );
        REQUIRE( parser.units().prefix().size() >= 1 );

        CHECK( parser.units().ctOrder()[0] == CtElement::V );
        CHECK( parser.units().ctOrder()[1] == CtElement::A );
        CHECK( parser.units().ctOrder()[2] == CtElement::D );

        CHECK( parser.units().prefix()[0] == "hello" );
        CHECK( parser.units().dUnit() == Length::feet() );
        CHECK( parser.units().sUnit() == Length::feet() );

        parser.parseLine("#units $hello $world");
        CHECK( parser.macros()["hello"] == "" );
        CHECK( parser.macros()["world"] == "" );

        CHECK_THROWS( parser.parseLine("#units $(undefined)") );
    }
}

TEST_CASE( "WallsUnits tests", "[dewalls]" ) {
    WallsUnits units;

    SECTION( "avgInc" ) {
        units.setTypevbCorrected(true);
        CHECK( units.avgInc(UAngle(3, Angle::degrees()), UAngle(1, Angle::degrees())) == UAngle(2, Angle::degrees()) );
        CHECK( units.avgInc(UAngle(1, Angle::degrees()), UAngle(3, Angle::degrees())) == UAngle(2, Angle::degrees()) );
        CHECK( units.avgInc(UAngle(3, Angle::degrees()), UAngle()) == UAngle(3, Angle::degrees()) );
        CHECK( units.avgInc(UAngle(), UAngle(3, Angle::degrees())) == UAngle(3, Angle::degrees()) );

        units.setTypevbCorrected(false);
        CHECK( units.avgInc(UAngle(3, Angle::degrees()), UAngle(-1, Angle::degrees())) == UAngle(2, Angle::degrees()) );
        CHECK( units.avgInc(UAngle(1, Angle::degrees()), UAngle(-3, Angle::degrees())) == UAngle(2, Angle::degrees()) );
        CHECK( units.avgInc(UAngle(), UAngle(3, Angle::degrees())) == UAngle(-3, Angle::degrees()) );
    }

    SECTION( "applyHeightCorrections" ) {
        units.setTypevbCorrected(true);
        ULength dist(sqrt(2) * 5 - 1, Length::meters());
        units.setIncd(ULength(1, Length::meters()));
        UAngle fsInc(42, Angle::degrees());
        units.setIncv(UAngle(1, Angle::degrees()));
        UAngle bsInc(49, Angle::degrees());
        units.setIncvb(UAngle(-2, Angle::degrees()));
        units.setInch(ULength(2, Length::meters()));
        ULength ih(-8, Length::meters());
        ULength th(-1, Length::meters());

        units.applyHeightCorrections(dist, fsInc, bsInc, ih, th);

        CHECK( uabs(dist - ULength(4, Length::meters())) < ULength(1e-6, Length::meters()) );
        CHECK( uabs(fsInc - UAngle(-3, Angle::degrees())) < UAngle(1e-6, Angle::degrees()) );
        CHECK( uabs(bsInc - UAngle(4, Angle::degrees())) < UAngle(1e-6, Angle::degrees()) );

        dist = ULength(5, Length::meters());
        fsInc = UAngle(90, Angle::degrees());
        bsInc = UAngle();
    }
}
