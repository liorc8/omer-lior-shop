#include "DrawThread.h"
#include "GuiMain.h"
#include "../../shared/ImGuiSrc/imgui.h"  // Includes the ImGui library for GUI functionality
#include <algorithm>  // Includes the algorithm library for standard algorithms
#include <cctype>  // Includes the cctype library for character handling functions
#include <filesystem>  // Includes the filesystem library for file system operations
#include <fstream>  // Includes the fstream library for file input/output
#include <sstream>  // Includes the sstream library for string stream operations
#include <cstring>  // Includes the cstring library for C-style string handling
#include <iostream>  // Includes the iostream library for standard input/output
#include <regex>  // Includes the regex library for regular expressions

namespace fs = std::filesystem;  // Creates an alias for the filesystem namespace

// Utility function to convert a string to lowercase
std::string toLower(const std::string& str) {
    std::string lower_str = str;  // Copy the input string to a new string
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), [](unsigned char c) { return std::tolower(c); });
    // Convert each character in the string to lowercase using std::transform
    return lower_str;  // Return the converted lowercase string
}

// Function to save an order to a file
void SaveOrder(const CommonObjects& common, const std::string& fullName, const std::string& address, float totalPrice) {
    static int orderCounter = 1;  // Static variable to keep track of order numbers
    fs::create_directories("orders");  // Create the "orders" directory if it doesn't exist
    std::string orderFileName = "orders/order" + std::to_string(orderCounter++) + ".txt";  // Generate the order file name
    std::ofstream orderFile(orderFileName);  // Open the file for writing

    if (!orderFile.is_open()) {  // Check if the file was successfully opened
        std::cerr << "Failed to open order file: " << orderFileName << std::endl;  // Print an error message if the file couldn't be opened
        return;  // Exit the function
    }

    orderFile << "Full Name: " << fullName << "\n";  // Write the full name to the order file
    orderFile << "Address: " << address << "\n";  // Write the address to the order file
    orderFile << "Items:\n";  // Write the header for the items section
    for (const auto& item : common.cart) {  // Iterate over the items in the cart
        orderFile << item.product.title << " x " << item.quantity << " - $" << item.product.price * item.quantity << "\n";  // Write each item to the order file
    }
    orderFile << "Total Price: $" << totalPrice << "\n";  // Write the total price to the order file
    orderFile.close();  // Close the order file

    std::cout << "Order saved successfully: " << orderFileName << std::endl;  // Print a success message
}

// Function to save customer details to a file
void SaveCustomer(const std::string& id, const std::string& fullName, const std::string& address, const std::string& creditCard, const std::string& expiry, const std::string& cvv) {
    fs::create_directories("customers");  // Create the "customers" directory if it doesn't exist
    std::string customerFileName = "customers/" + id + ".txt";  // Generate the customer file name
    std::ofstream customerFile(customerFileName);  // Open the file for writing

    if (!customerFile.is_open()) {  // Check if the file was successfully opened
        std::cerr << "Failed to open customer file: " << customerFileName << std::endl;  // Print an error message if the file couldn't be opened
        return;  // Exit the function
    }

    customerFile << "ID: " << id << "\n";  // Write the customer ID to the file
    customerFile << "Full Name: " << fullName << "\n";  // Write the full name to the file
    customerFile << "Address: " << address << "\n";  // Write the address to the file
    customerFile << "Credit Card: " << creditCard << "\n";  // Write the credit card number to the file
    customerFile << "Expiry: " << expiry << "\n";  // Write the credit card expiry date to the file
    customerFile << "CVV: " << cvv << "\n";  // Write the credit card CVV to the file
    customerFile.close();  // Close the customer file

    std::cout << "Customer saved successfully: " << customerFileName << std::endl;  // Print a success message
}

// Input validation functions

// Function to validate an ID (must be exactly 9 digits)
bool isValidID(const char* id) {
    return std::regex_match(id, std::regex("^[0-9]{9}$"));  // Check if the ID matches the regex pattern
}

// Function to validate a name (must contain only letters and spaces)
bool isValidName(const char* name) {
    return std::regex_match(name, std::regex("^[A-Za-z ]+$"));  // Check if the name matches the regex pattern
}

// Function to validate a credit card number (must be exactly 16 digits)
bool isValidCreditCard(const char* creditCard) {
    return std::regex_match(creditCard, std::regex("^[0-9]{16}$"));  // Check if the credit card number matches the regex pattern
}

// Function to validate an expiry date (must be exactly 4 digits: MMYY)
bool isValidExpiry(const char* expiry) {
    return std::regex_match(expiry, std::regex("^[0-9]{4}$"));  // Check if the expiry date matches the regex pattern
}

// Function to validate a CVV (must be exactly 3 digits)
bool isValidCVV(const char* cvv) {
    return std::regex_match(cvv, std::regex("^[0-9]{3}$"));  // Check if the CVV matches the regex pattern
}

bool showErrorPopup = false;  // Flag to control the display of an error popup
std::string errorMessage = "";  // Stores the error message to be displayed in the popup

// Function to load customer details from a file
bool LoadCustomer(const std::string& id, std::string& fullName, std::string& address, std::string& creditCard, std::string& expiry, std::string& cvv) {
    std::string customerFileName = "customers/" + id + ".txt";  // Generate the customer file name
    std::ifstream customerFile(customerFileName);  // Open the customer file for reading

    if (!customerFile.is_open()) {  // Check if the file was successfully opened
        showErrorPopup = true;  // Set the flag to show the error popup if the file couldn't be opened
        errorMessage = "Error: Customer file not found!";  // Set the error message
        return false;  // Return false to indicate that the customer was not found
    }

    std::string line;
    while (std::getline(customerFile, line)) {  // Read each line from the file
        std::istringstream iss(line);
        std::string key;
        if (std::getline(iss, key, ':')) {  // Split the line into a key and value
            std::string value;
            if (std::getline(iss, value)) {
                value.erase(0, value.find_first_not_of(" \t"));  // Trim leading whitespace from the value
                if (key == "ID") {
                    // Skip, we already have the ID
                }
                else if (key == "Full Name") {
                    fullName = value;  // Store the full name
                }
                else if (key == "Address") {
                    address = value;  // Store the address
                }
                else if (key == "Credit Card") {
                    creditCard = value;  // Store the credit card number
                }
                else if (key == "Expiry") {
                    expiry = value;  // Store the expiry date
                }
                else if (key == "CVV") {
                    cvv = value;  // Store the CVV
                }
            }
        }
    }
    customerFile.close();  // Close the customer file
    return true;  // Return true to indicate success
}

// Function to draw the main application window
void DrawAppWindow(void* common_ptr) {
    auto common = static_cast<CommonObjects*>(common_ptr);  // Cast the input pointer to a CommonObjects pointer

    // Set the window to fill the entire screen and make it non-movable
    ImGui::SetNextWindowPos(ImVec2(0, 0));  // Set the window position to the top-left corner of the screen
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);  // Set the window size to cover the entire screen
    ImGui::Begin("Omer & Lior Shop", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    // Begin a new ImGui window with custom flags to make it non-movable, non-resizable, and without a title bar

    // Custom styling with new colors
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.2f, 1.0f));  // Set a dark blue background color
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));  // Set a bright orange color for buttons
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.4f, 0.0f, 1.0f));  // Set a dark orange color for buttons when hovered
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.8f, 0.0f, 1.0f));  // Set a bright yellow color for buttons when active
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));  // Set a dark gray background color for input frames
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);  // Set rounded corners for input frames
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));  // Set padding for the window content

    // Use the large font defined in GuiMain for the header only
    ImGuiIO& io = ImGui::GetIO();  // Get the ImGuiIO object for input/output operations
    ImFont* largeFont = io.Fonts->Fonts.back();  // Get the last loaded font (assumed to be the large font)
    ImFont* regularFont = io.Fonts->Fonts[0];   // Get the first loaded font (assumed to be the regular font)

    // Create a header area with new design
    ImGui::BeginChild("Header", ImVec2(0, 80), true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);//making the haedline
    // Begin a child window for the header with a fixed height of 80

    ImGui::PushFont(largeFont);  // Use the large font for the header text
    // Center the text
    float textWidth = ImGui::CalcTextSize("Welcome to Omer & Lior Shop!").x;  // Calculate the width of the header text
    float textHeight = ImGui::CalcTextSize("Welcome to Omer & Lior Shop!").y;  // Calculate the height of the header text
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - textWidth) * 0.5f - 75);  // Set the X position of the cursor to center the text, with a left offset for the cart button
    ImGui::SetCursorPosY((ImGui::GetWindowHeight() - textHeight) * 0.5f);  // Set the Y position of the cursor to center the text vertically
    ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.2f, 1.0f), "Welcome to Omer & Lior Shop!");  // Render the header text with a stylish orange color
    ImGui::PopFont();  // Restore the regular font

    // Adjust cart button position to align with the title
    ImGui::SameLine(ImGui::GetWindowWidth() - 150);  // Position the cart button on the same line as the header text, aligned to the right
    ImGui::SetCursorPosY((ImGui::GetWindowHeight() - 50) * 0.5f);  // Align the cart button vertically to the center of the header
    if (ImGui::Button("View Cart", ImVec2(120, 50))) {  // Render the "View Cart" button with custom size
        common->m_window_shown = WindowShown::CART;  // If the button is clicked, switch to the cart window
    }

    ImGui::EndChild();  // End the header child window

    // Create a main content area with product cards
    ImGui::BeginChild("MainContent", ImVec2(0, 0), true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
    // Begin a child window for the main content area

    ImGui::PushFont(regularFont);  // Ensure the regular font is used for the rest of the content
    static char buff[200];  // Buffer for the search input text
    std::string search_text = toLower(buff);  // Convert the search text to lowercase

    if (common->data_ready) {  // Check if the data is ready to be displayed
        switch (common->m_window_shown) {  // Determine which window should be shown
        case WindowShown::ALL_PRODUCTS: {  // If the "All Products" window should be shown
            ImGui::InputTextWithHint("##search", "Search for products...", buff, sizeof(buff), ImGuiInputTextFlags_EnterReturnsTrue);  // Input field for product search
            ImGui::Separator();  // Draw a separator line

            // Begin the table for product cards
            ImGui::BeginTable("ProductTable", 1, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg);  // Begin a table with one column
            ImGui::TableNextRow();  // Move to the next row in the table
            ImGui::TableSetColumnIndex(0);  // Set the current column index to 0

            for (const auto& rec : common->products) {  // Iterate over the products
                if (search_text.empty() || toLower(rec.title).find(search_text) != std::string::npos) {  // Check if the product matches the search text
                    ImGui::PushID(rec.id);  // Push a unique identifier for the product
                    ImGui::BeginGroup();  // Begin a group for the product card
                    ImGui::BeginChild("ProductCard", ImVec2(0, 150), true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
                    // Begin a child window for the product card with a fixed height of 150
                    ImGui::Text(rec.title.c_str());  // Display the product title
                    ImGui::Text("Price: $%.2f", rec.price);  // Display the product price
                    ImGui::Spacing();  // Add some spacing
                    if (ImGui::Button("View Details", ImVec2(120, 40))) {  // Display a "View Details" button for the product
                        common->selected_product_index = rec.id - 1;  // Set the selected product index
                        common->m_window_shown = WindowShown::SPECIFIC_PRODUCT;  // Switch to the specific product window
                        common->download_needed = true;  // Set the flag to download additional data
                        common->cv.notify_one();  // Notify the download thread to start downloading
                    }
                    ImGui::EndChild();  // End the product card child window
                    ImGui::EndGroup();  // End the product card group
                    ImGui::PopID();  // Pop the unique identifier for the product
                    ImGui::Spacing();  // Add some spacing
                }
            }

            ImGui::EndTable();  // End the table
            break;
        }
        case WindowShown::SPECIFIC_PRODUCT: {  // If the "Specific Product" window should be shown
            if (common->selected_product_index >= 0 && common->selected_product_index < common->products.size()) {  // Check if a valid product is selected
                auto& rec = common->products[common->selected_product_index];  // Get the selected product
                ImGui::Text("Title: %s", rec.title.c_str());  // Display the product title
                ImGui::Separator();  // Draw a separator line
                ImGui::TextWrapped("Description: %s", rec.description.c_str());  // Display the product description
                ImGui::Separator();  // Draw a separator line
                ImGui::Text("Category: %s", rec.category.c_str());  // Display the product category
                ImGui::Separator();  // Draw a separator line
                ImGui::Text("Price: $%.2f", rec.price);  // Display the product price
                ImGui::Separator();  // Draw a separator line
                ImGui::Text("Stock: %d", rec.stock);  // Display the product stock quantity
                ImGui::Separator();  // Draw a separator line
                ImGui::Text("Rating: %.2f", rec.rating);  // Display the product rating
                ImGui::Separator();  // Draw a separator line
                ImGui::Text("Return Policy: %s", rec.returnPolicy.c_str());  // Display the product return policy
                ImGui::Separator();  // Draw a separator line
                ImGui::Text("Reviews:");  // Display the reviews header
                ImGui::Separator();  // Draw a separator line

                for (const auto& review : rec.reviews) {  // Iterate over the product reviews
                    ImGui::Text("Reviewer name: %s", review.reviewerName.c_str());  // Display the reviewer's name
                    ImGui::Separator();  // Draw a separator line
                    ImGui::Text("Review date: %s", review.date.c_str());  // Display the review date
                    ImGui::Separator();  // Draw a separator line
                    ImGui::Text("Reviewer email: %s", review.reviewerEmail.c_str());  // Display the reviewer's email
                    ImGui::Separator();  // Draw a separator line
                    ImGui::Text("Reviewer comment: %s", review.comment.c_str());  // Display the reviewer's comment
                    ImGui::Separator();  // Draw a separator line
                    ImGui::Text("Reviewer rating: %d", review.rating);  // Display the reviewer's rating
                    ImGui::Separator();  // Draw a separator line
                }

                bool atMaxStock = false;  // Flag to check if the maximum stock is reached
                for (const auto& item : common->cart) {  // Iterate over the cart items
                    if (item.product.id == rec.id && item.quantity >= rec.stock) {  // Check if the cart already has the maximum stock for this product
                        atMaxStock = true;  // Set the flag if the maximum stock is reached
                        break;  // Exit the loop
                    }
                }

                if (atMaxStock) {  // If the maximum stock is reached
                    ImGui::Text("Maximum stock taken");  // Display a message indicating the maximum stock is reached
                }
                else if (rec.stock > 0) {  // If the product is in stock
                    if (ImGui::Button("Add to Cart", ImVec2(120, 40))) {  // Display an "Add to Cart" button
                        common->AddToCart(rec, 1);  // Add the product to the cart with a quantity of 1
                    }
                }
                else {  // If the product is out of stock
                    ImGui::Text("Out of Stock");  // Display an "Out of Stock" message
                }

                if (ImGui::Button("Back to all products", ImVec2(200, 40))) {  // Display a "Back to all products" button with custom size
                    common->m_window_shown = WindowShown::ALL_PRODUCTS;  // Switch back to the all products window
                    common->cv.notify_one();  // Notify the download thread to start downloading
                }

                auto it = common->downloaded_images_paths.find(common->selected_product_index);  // Find the downloaded images for the selected product
                if (it != common->downloaded_images_paths.end()) {  // If there are downloaded images
                    for (const auto& image_path : it->second) {  // Iterate over the image paths
                        ImGui::Text("An image has been downloaded to %s!", image_path.c_str());  // Display a message indicating the image was downloaded
                    }
                }
            }
            break;
        }
        case WindowShown::CART: {  // If the "Cart" window should be shown
            ImGui::Text("Shopping Cart:");  // Display the cart header
            ImGui::Separator();  // Draw a separator line
            float total_price = 0.0f;  // Initialize the total price to 0
            if (common->cart.empty()) {  // Check if the cart is empty
                ImGui::Text("The cart is empty");  // Display a message indicating the cart is empty
            }
            else {  // If the cart is not empty
                for (const auto& item : common->cart) {  // Iterate over the cart items
                    ImGui::Text("Title: %s", item.product.title.c_str());  // Display the product title
                    ImGui::Text("Quantity: %d", item.quantity);  // Display the quantity of the product
                    ImGui::Text("Price: %.2f$", item.product.price * item.quantity);  // Display the total price for this product
                    total_price += item.product.price * item.quantity;  // Add the product price to the total price

                    ImGui::PushID(item.product.id);  // Push a unique identifier for the product
                    if (ImGui::Button("Remove", ImVec2(120, 40))) {  // Display a "Remove" button
                        common->DecreaseQuantityInCart(item.product.id);  // Decrease the quantity of the product in the cart
                    }
                    ImGui::PopID();  // Pop the unique identifier for the product

                    ImGui::Separator();  // Draw a separator line
                }
                ImGui::Text("Total Price: %.2f$", total_price);  // Display the total price of the cart

                if (ImGui::Button("Checkout", ImVec2(120, 40))) {  // Display a "Checkout" button
                    common->m_window_shown = WindowShown::CHECKOUT;  // Switch to the checkout window
                }
            }
            if (ImGui::Button("Back to all products", ImVec2(200, 40))) {  // Display a "Back to all products" button with custom size
                common->m_window_shown = WindowShown::ALL_PRODUCTS;  // Switch back to the all products window
            }
            break;
        }
        case WindowShown::CHECKOUT: {  // If the "Checkout" window should be shown
            static char fullName[128] = "";  // Buffer for the user's full name
            static char address[256] = "";  // Buffer for the user's address
            static char creditCard[17] = "";  // Buffer for the user's credit card number
            static char expiry[5] = "";  // Buffer for the credit card expiry date
            static char cvv[4] = "";  // Buffer for the credit card CVV code
            static char id[10] = "";  // Buffer for the user's ID
            static bool saveDetails = false;  // Flag to indicate if the user's details should be saved
            static bool existingCustomer = false;  // Flag to indicate if the user is an existing customer
            static char existingId[10] = "";  // Buffer for the existing customer's ID

            ImGui::Checkbox("Existing Customer", &existingCustomer);  // Checkbox for existing customers
            if (existingCustomer) {  // If the user is an existing customer
                ImGui::InputText("ID", existingId, sizeof(existingId));  // Input field for the existing customer's ID
                if (ImGui::Button("Load Customer", ImVec2(120, 40))) {  // Display a "Load Customer" button
                    std::string addr, cc, exp, cv, identity, name;
                    if (LoadCustomer(existingId, name, addr, cc, exp, cv)) {  // Load the customer details from the file
                        strncpy_s(id, existingId, sizeof(id));  // Copy the ID to the input buffer
                        strncpy_s(fullName, name.c_str(), sizeof(fullName));  // Copy the full name to the input buffer
                        strncpy_s(address, addr.c_str(), sizeof(address));  // Copy the address to the input buffer
                        strncpy_s(creditCard, cc.c_str(), sizeof(creditCard));  // Copy the credit card number to the input buffer
                        strncpy_s(expiry, exp.c_str(), sizeof(expiry));  // Copy the expiry date to the input buffer
                        strncpy_s(cvv, cv.c_str(), sizeof(cvv));  // Copy the CVV code to the input buffer
                    }
                    else {
                        showErrorPopup = true;  // Set the flag to show the error popup if the customer file is not found
                    }
                }
            }
            else {  // If the user is a new customer
                ImGui::InputText("ID", id, sizeof(id));  // Input field for the user's ID
            }

            ImGui::InputText("Full Name", fullName, sizeof(fullName));  // Input field for the user's full name
            ImGui::InputText("Address", address, sizeof(address));  // Input field for the user's address
            ImGui::InputText("Credit Card Number", creditCard, sizeof(creditCard));  // Input field for the user's credit card number
            ImGui::InputText("Expiry Date", expiry, sizeof(expiry));  // Input field for the credit card expiry date
            ImGui::InputText("CVV", cvv, sizeof(cvv));  // Input field for the credit card CVV code
            ImGui::Checkbox("Save details for next purchase", &saveDetails);  // Checkbox to save the user's details for future purchases

            // Check if all fields are filled out
            bool allFieldsFilled = strlen(fullName) > 0 &&
                strlen(address) > 0 &&
                strlen(creditCard) > 0 &&
                strlen(expiry) > 0 &&
                strlen(cvv) > 0 &&
                strlen(id) > 0;

            if (allFieldsFilled && ImGui::Button("Pay", ImVec2(120, 50))) {  // Display a "Pay" button if all fields are filled out
                if (!isValidID(id)) {  // Validate the ID
                    errorMessage = "Invalid ID: Must be exactly 9 digits.";  // Set the error message for an invalid ID
                    showErrorPopup = true;  // Set the flag to show the error popup
                }
                else if (!isValidName(fullName)) {  // Validate the full name
                    errorMessage = "Invalid Name: Must contain only letters.";  // Set the error message for an invalid name
                    showErrorPopup = true;  // Set the flag to show the error popup
                }
                else if (!isValidCreditCard(creditCard)) {  // Validate the credit card number
                    errorMessage = "Invalid Credit Card: Must be exactly 16 digits.";  // Set the error message for an invalid credit card number
                    showErrorPopup = true;  // Set the flag to show the error popup
                }
                else if (!isValidExpiry(expiry)) {  // Validate the expiry date
                    errorMessage = "Invalid Expiry: Must be exactly 4 digits.";  // Set the error message for an invalid expiry date
                    showErrorPopup = true;  // Set the flag to show the error popup
                }
                else if (!isValidCVV(cvv)) {  // Validate the CVV code
                    errorMessage = "Invalid CVV: Must be exactly 3 digits.";  // Set the error message for an invalid CVV code
                    showErrorPopup = true;  // Set the flag to show the error popup
                }
                else {  // If all validations pass
                    float total_price = 0.0f;  // Initialize the total price to 0
                    for (const auto& item : common->cart) {  // Iterate over the cart items
                        total_price += item.product.price * item.quantity;  // Calculate the total price of the cart
                    }

                    SaveOrder(*common, fullName, address, total_price);  // Save the order to a file
                    if (saveDetails) {  // If the user wants to save their details
                        SaveCustomer(id, fullName, address, creditCard, expiry, cvv);  // Save the customer details to a file
                    }

                    for (auto& item : common->cart) {  // Iterate over the cart items
                        for (auto& product : common->products) {  // Iterate over the products
                            if (product.id == item.product.id) {  // Find the product that matches the cart item
                                product.stock -= item.quantity;  // Decrease the stock quantity of the product
                                break;  // Exit the loop
                            }
                        }
                    }

                    common->cart.clear();  // Clear the cart after checkout
                    common->m_window_shown = WindowShown::THANK_YOU;  // Redirect to the "Thank You" screen

                    memset(fullName, 0, sizeof(fullName));  // Clear the full name input buffer
                    memset(address, 0, sizeof(address));  // Clear the address input buffer
                    memset(creditCard, 0, sizeof(creditCard));  // Clear the credit card input buffer
                    memset(expiry, 0, sizeof(expiry));  // Clear the expiry date input buffer
                    memset(cvv, 0, sizeof(cvv));  // Clear the CVV input buffer
                    memset(id, 0, sizeof(id));  // Clear the ID input buffer
                    memset(existingId, 0, sizeof(existingId));  // Clear the existing customer ID input buffer
                    saveDetails = false;  // Reset the save details flag
                    existingCustomer = false;  // Reset the existing customer flag
                }
            }

            if (ImGui::Button("Back to cart", ImVec2(120, 40))) {  // Display a "Back to cart" button
                common->m_window_shown = WindowShown::CART;  // Switch back to the cart window
            }
            break;
        }
        case WindowShown::THANK_YOU: {  // If the "Thank You" window should be shown
            ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.2f, 1.0f), "Thank you for shopping at Omer & Lior Shop. See you next time!");  // Display a thank you message with a stylish color

            ImGui::Spacing();  // Add some spacing
            ImGui::Spacing();  // Add some more spacing
            ImGui::Spacing();  // Add even more spacing

            if (ImGui::Button("Back to Home", ImVec2(200, 60))) {  // Display a "Back to Home" button with custom size
                common->m_window_shown = WindowShown::ALL_PRODUCTS;  // Switch back to the all products window
            }
            break;
        }
        }
    }

    ImGui::PopFont();  // Restore the regular font after usage
    ImGui::EndChild();  // End the main content child window

    ImGui::PopStyleVar(2);  // Pop the style variables for rounded corners and padding
    ImGui::PopStyleColor(5);  // Pop the color styles

    ImGui::End();  // End the main window

    // Handle error popup for missing customer file
    if (showErrorPopup) {  // If the flag is set to show the error popup
        ImGui::OpenPopup("Error");  // Open the error popup
        showErrorPopup = false;  // Reset the flag to prevent repeated popup opening
    }

    // Custom popup style with the same theme
    if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {  // Begin the error popup modal
        // Custom styling for the popup window
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.2f, 1.0f));  // Set a dark blue background color
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));  // Set a bright orange color for buttons
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.4f, 0.0f, 1.0f));  // Set a dark orange color for buttons when hovered
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.8f, 0.0f, 1.0f));  // Set a bright yellow color for buttons when active
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);  // Set rounded corners for the popup window
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));  // Set padding for the popup window content

        ImGui::Text("%s", errorMessage.c_str());  // Display the dynamic error message

        if (ImGui::Button("OK", ImVec2(120, 0))) {  // Display an "OK" button
            ImGui::CloseCurrentPopup();  // Close the error popup when the button is clicked
        }

        // Pop custom popup styles
        ImGui::PopStyleVar(2);  // Pop the style variables for rounded corners and padding
        ImGui::PopStyleColor(4);  // Pop the color styles

        ImGui::EndPopup();  // End the error popup modal
    }
}

// Function that the DrawThread runs, which continuously draws the GUI
void DrawThread::operator()(CommonObjects& common) {
    GuiMain(DrawAppWindow, &common);  // Call the main GUI function and pass the DrawAppWindow function to be used for rendering
    common.exit_flag = true;  // Set the exit flag to true when the drawing is done
}
