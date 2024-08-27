// CommonObject.cpp
#include "CommonObject.h"
#include <algorithm>

// Adds a product to the cart
void CommonObjects::AddToCart(const Product& product, int quantity) {
    //std::lock_guard<std::mutex> lock(mtx);  // Ensures thread-safe access to the cart
    for (auto& item : cart) {
        if (item.product.id == product.id) {  // Checks if the product is already in the cart
            if (item.quantity + quantity <= product.stock) {
                item.quantity += quantity;  // Increases quantity if within stock limits
            }
            else {
                item.quantity = product.stock;  // Limits to maximum available stock
            }
            return;  // Exits after updating the quantity
        }
    }
    if (quantity <= product.stock) {
        cart.push_back({ product, quantity });  // Adds new item to the cart if within stock limits
    }
    else {
        cart.push_back({ product, product.stock });  // Adds item with maximum available stock
    }
}

// Removes a product from the cart
void CommonObjects::RemoveFromCart(int product_id) {
    std::lock_guard<std::mutex> lock(mtx);  // Ensures thread-safe access to the cart
    cart.erase(std::remove_if(cart.begin(), cart.end(), [product_id](const CartItem& item) {
        return item.product.id == product_id;  // Finds and removes the product by ID
        }), cart.end());
}

// Decreases the quantity of a product in the cart
void CommonObjects::DecreaseQuantityInCart(int product_id) {
    std::lock_guard<std::mutex> lock(mtx);  // Ensures thread-safe access to the cart
    for (auto it = cart.begin(); it != cart.end(); ++it) {
        if (it->product.id == product_id) {  // Finds the product by ID
            if (it->quantity > 1) {
                it->quantity--;  // Decreases the quantity if greater than 1
            }
            else {
                cart.erase(it);  // Removes the item if the quantity is 1
            }
            return;  // Exits after updating or removing the item
        }
    }
}
