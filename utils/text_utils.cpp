#include "text_utils.hpp"

std::string getColumnLetter(int index) {
    if (index >= 0 && index < 26) {
        return std::string(1, 'A' + index);
    }
    return "?";
}