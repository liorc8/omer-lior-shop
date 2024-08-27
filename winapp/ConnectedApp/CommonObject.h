// CommonObject.h
#pragma once  // Prevents multiple inclusions of the same header file.

#include <atomic>  // Provides atomic operations for thread-safe operations.
#include <string>  // Provides the string class.
#include <vector>  // Provides the vector container class.
#include <condition_variable>  // Provides condition variables for thread synchronization.
#include <mutex>  // Provides mutexes for thread synchronization.
#include <unordered_map>  // Provides the unordered_map container class.

#define CPPHTTPLIB_OPENSSL_SUPPORT  // Enables OpenSSL support in httplib.
#include "httplib.h"  // Includes the httplib library for HTTP functionality.
#include "stb_image.h"  // Includes the stb_image library for image loading.

struct Review {
    int rating;  // Rating of the product, typically out of 5.
    std::string comment;  // Comment left by the reviewer.
    std::string date;  // Date when the review was posted.
    std::string reviewerName;  // Name of the reviewer.
    std::string reviewerEmail;  // Email of the reviewer.
};

struct Product {
    int id;  // Unique identifier for the product.
    std::string title;  // Title of the product.
    std::string description;  // Detailed description of the product.
    std::string category;  // Category to which the product belongs.
    std::string returnPolicy;  // Return policy for the product.
    float price;  // Price of the product.
    float rating;  // Average rating of the product.
    int stock;  // Number of items available in stock.
    std::vector<std::string> tags;  // Tags associated with the product for easy search.
    std::vector<std::string> images;  // List of image URLs associated with the product.
    std::string thumbnail;  // URL of the product's thumbnail image.
    std::vector<Review> reviews;  // List of reviews associated with the product.
};

struct CartItem {
    Product product;  // The product that is added to the cart.
    int quantity;  // The quantity of the product in the cart.
};

enum WindowShown {
    ALL_PRODUCTS,  // Enum value representing the window showing all products.
    SPECIFIC_PRODUCT,  // Enum value representing the window showing a specific product.
    CART,  // Enum value representing the cart window.
    CHECKOUT,  // Enum value representing the checkout window.
    THANK_YOU  // Enum value representing the thank you window after purchase.
};

struct CommonObjects {
    std::vector<Product> products;  // List of all products available in the store.
    std::vector<CartItem> cart;  // Shopping cart containing products selected by the user.

    std::string url;  // URL of the server or API endpoint.
    bool data_ready = false;  // Flag to indicate if data has been loaded and is ready for use.
    int selected_product_index = -1;  // Index of the currently selected product in the products list.
    WindowShown m_window_shown = WindowShown::ALL_PRODUCTS;  // Enum value representing the currently shown window.
    std::atomic<bool> exit_flag{ false };  // Atomic flag to signal when the application should exit.

    std::mutex mtx;  // Mutex for thread-safe operations.
    std::condition_variable cv;  // Condition variable for thread synchronization.
    bool download_needed = true;  // Flag to indicate if a download is needed.

    std::unordered_map<int, std::vector<std::string>> downloaded_images_paths;  // Maps product IDs to lists of downloaded image paths.

    void AddToCart(const Product& product, int quantity);  // Adds a product to the cart with the specified quantity.
    void RemoveFromCart(int product_id);  // Removes a product from the cart by its ID.
    void DecreaseQuantityInCart(int product_id);  // Decreases the quantity of a product in the cart by its ID.
};
