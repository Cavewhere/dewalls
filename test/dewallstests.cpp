#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_translate_exception.hpp>

#include "segmentparseexception.h"

CATCH_TRANSLATE_EXCEPTION( dewalls::SegmentParseException const& ex ) {
    return ex.message().toStdString();
}
