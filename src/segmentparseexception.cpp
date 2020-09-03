#include "segmentparseexception.h"
#include "wallsmessage.h"

//Std includes
#include <sstream>

namespace dewalls {

QString SegmentParseException::message() const
{
    return WallsMessage(*this).toString();
}

} // namespace dewalls

