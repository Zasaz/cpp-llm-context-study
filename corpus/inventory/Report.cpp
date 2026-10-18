#include "Report.h"

Report::Report() {
}

Report::Report(const Warehouse& warehouse) {
    source = warehouse;
}

double Report::averageItemValue() const {
    int count = source.itemCount();
    if (count == 0) {
        return 0.0;
    }
    return source.inventoryValue() / static_cast<double>(count);
}

int Report::outOfStockCount() const {
    int total = 0;
    for (unsigned int i = 0; i < source.items.size(); ++i) {
        if (!source.items[i].isInStock()) {
            total = total + 1;
        }
    }
    return total;
}

bool Report::isEmpty() const {
    return source.itemCount() == 0;
}

std::string Report::summaryLine() const {
    if (isEmpty()) {
        return "empty";
    }
    return source.label;
}
