#include "../lib/catch.hpp"

#include "../src/wallsprojectparser.h"

#include <iostream>

using namespace dewalls;

TEST_CASE( "WallsProjectParserTests", "[WallsProjectParser]" ) {
   WallsProjectParser parser;

   QObject::connect(&parser,
                    static_cast<void(WallsProjectParser::*)(Severity,QString)>(&WallsProjectParser::message),
                    [=](const Severity& severity, const QString& message) {
       Q_UNUSED(severity);
       std::cout << message.toStdString() << std::endl;
   });
   QObject::connect(&parser,
                    static_cast<void(WallsProjectParser::*)(Severity,QString,QString)>(&WallsProjectParser::message),
                    [=](const Severity& severity, const QString& message, const QString& source) {
       Q_UNUSED(severity);
       std::cout << "In file: " << source.toStdString() << std::endl;
       std::cout << message.toStdString() << std::endl;
   });
   QObject::connect(&parser,
                    static_cast<void(WallsProjectParser::*)(Severity,QString,QString,int,int,int,int)>(
                        &WallsProjectParser::message),
                    [=](const Severity& severity, const QString& message, const QString& source,
                        int startLine, int startColumn, int endLine, int endColumn) {
       Q_UNUSED(severity);
       Q_UNUSED(startColumn);
       Q_UNUSED(endLine);
       Q_UNUSED(endColumn);
       std::cout << "In file: " << source.toStdString() << std::endl;
       std::cout << "In file: " << source.toStdString() << std::endl;
       std::cout << "In file: " << source.toStdString() << std::endl;
       std::cout << "Line " << startLine << std::endl;
       std::cout << message.toStdString() << std::endl;
   });

   WpjBookPtr projectRoot;

   projectRoot = parser.parseFile(":/test/Kaua North Maze.wpj");

   REQUIRE( !projectRoot.isNull() );
   CHECK( projectRoot->Title == "Actun Kaua - North Maze" );
   CHECK( projectRoot->Name == "KAUA-NM" );

   REQUIRE( !projectRoot->reference().isNull() );
   CHECK( projectRoot->reference()->northing == 2308521.655 );
   CHECK( projectRoot->reference()->easting == 324341.706 );
   CHECK( projectRoot->reference()->zone == 16 );
   CHECK( projectRoot->reference()->gridConvergence == -0.602 );
   CHECK( projectRoot->reference()->elevation == 27 );

   REQUIRE( projectRoot->Children.size() == 5 );

   WpjEntryPtr child0 = projectRoot->Children[0];
   CHECK( child0->Title == "SVG Map Info - Please Read First!" );
   CHECK( child0->Name == "SVGINFO.TXT" );
   CHECK( child0->isOther() );

   WpjEntryPtr child1 = projectRoot->Children[1];
   CHECK( child1->Title == "Flags and Notes" );
   CHECK( child1->Name == "NOTES" );
   CHECK( child1->isSurvey() );

   WpjEntryPtr child2 = projectRoot->Children[2];
   CHECK( child2->Title == "SVG Sources" );
   CHECK( child2->Name == QString() );
   CHECK( child2->isBook() );

   WpjEntryPtr child3 = projectRoot->Children[3];
   CHECK( child3->Title == "Fixed Points" );
   CHECK( child3->Name == QString() );
   CHECK( child3->isBook() );

   WpjEntryPtr child4 = projectRoot->Children[4];
   CHECK( child4->Title == "North Maze Surveys" );
   CHECK( child4->Name == "NORTH" );
   CHECK( child4->isBook() );
}
