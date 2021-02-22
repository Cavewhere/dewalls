#include "length.h"

using namespace dewalls;

int main() {
    double meters = Length::convert(10.0, Length::Kilometers, Length::Meters);
    if(meters == 10.0 * 1000.0) {
        return 0;
    }
    return 1;
}
