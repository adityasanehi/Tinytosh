#ifndef SHOPIFY_SERVICE_H
#define SHOPIFY_SERVICE_H

#include "structs.h"

class ShopifyService {
public:
    bool fetchSales(const Config& config, ShopifyData &data);
};

#endif
