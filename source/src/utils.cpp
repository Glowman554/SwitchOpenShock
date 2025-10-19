#include "utils.hpp"

int findIndex(std::vector<std::string>& array, std::string goal) {
    for (int i = 0; i < array.size(); i++) {
        if (array[i] == goal) {
            return i;
        }
    }

    return -1;
}