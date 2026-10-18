#ifndef REPORT_H
#define REPORT_H

#include "Warehouse.h"
#include <string>

class Report {
public:
    Warehouse source;

    Report();
    Report(const Warehouse& warehouse);

    double averageItemValue() const;
    int outOfStockCount() const;
    std::string summaryLine() const;
    bool isEmpty() const;
};

#endif
