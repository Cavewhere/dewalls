#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "segmentparseexception.h"

CATCH_TRANSLATE_EXCEPTION( dewalls::SegmentParseException& ex ) {
    return ex.message().toLocal8Bit().constData();
}
