#include "../lib/catch.hpp"

#include "../src/wallsprojectparser.h"

#include <iostream>

using namespace dewalls;

typedef UnitizedDouble<Length> ULength;
typedef UnitizedDouble<Angle> UAngle;

TEST_CASE( "WallsProjectParserTests", "[WallsProjectParser]" ) {
    SECTION( "correctly parses valid file" ) {
        WallsProjectParser parser;

        QObject::connect(&parser, &WallsProjectParser::message,
                         [=](WallsMessage message) {
            std::cout << message.toString().toStdString() << std::endl;
        });

        WpjBookPtr projectRoot;

        projectRoot = parser.parseFile(":/test/Kaua North Maze.wpj");

        REQUIRE( !projectRoot.isNull() );
        CHECK( projectRoot->Title == "Actun Kaua - North Maze" );
        CHECK( projectRoot->Name.value() == "KAUA-NM" );
        CHECK( projectRoot->defaultViewAfterCompilation() == WpjEntry::NorthOrEast);
        CHECK( !projectRoot->preserveVertShotLength() );
        CHECK( !projectRoot->preserveVertShotOrientation() );
        CHECK( projectRoot->deriveDeclFromDate() );
        CHECK( projectRoot->gridRelative() );

        REQUIRE( !projectRoot->reference().isNull() );
        CHECK( projectRoot->reference()->northing == ULength(2308521.655, Length::meters()) );
        CHECK( projectRoot->reference()->easting == ULength(324341.706, Length::meters()) );
        CHECK( projectRoot->reference()->zone == 16 );
        CHECK( projectRoot->reference()->gridConvergence == UAngle(-0.602, Angle::degrees()) );
        CHECK( projectRoot->reference()->elevation == ULength(27, Length::meters()) );

        REQUIRE( projectRoot->Children.size() == 5 );

        WpjEntryPtr child0 = projectRoot->Children[0];
        CHECK( child0->Title == "SVG Map Info - Please Read First!" );
        CHECK( child0->Name.value() == "SVGINFO.TXT" );
        CHECK( child0->isOther() );
        CHECK( child0->launchOptions() == WpjEntry::Edit );

        WpjEntryPtr child1 = projectRoot->Children[1];
        CHECK( child1->Title == "Flags and Notes" );
        CHECK( child1->Name.value() == "NOTES" );
        CHECK( child1->isSurvey() );

        WpjEntryPtr child2 = projectRoot->Children[2];
        CHECK( child2->Title == "SVG Sources" );
        CHECK( child2->Name.value() == QString() );
        CHECK( child2->isBook() );

        WpjEntryPtr child3 = projectRoot->Children[3];
        CHECK( child3->Title == "Fixed Points" );
        CHECK( child3->Name.value() == QString() );
        CHECK( child3->isBook() );

        WpjEntryPtr child4 = projectRoot->Children[4];
        CHECK( child4->Title == "North Maze Surveys" );
        CHECK( child4->Name.value() == "NORTH" );
        CHECK( child4->isBook() );
    }
}
