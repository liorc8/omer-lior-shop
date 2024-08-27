#include "DownloadThread.h"
#include "nlohmann/json.hpp"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "stb_image.h"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Product, id, title, description, category, price, stock, rating, returnPolicy, images)
// Defines the mapping for JSON serialization and deserialization for the Product structure

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Review, rating, comment, date, reviewerName, reviewerEmail)
// Defines the mapping for JSON serialization and deserialization for the Review structure

#define HTTP_STATUS_SUCCESS (200)  // Defines the HTTP success status code

ParsedURL parse_url(const std::string& url) {
    const std::regex url_regex(R"(^https?://([^/]+)(/.*)?)");  // Regular expression to parse the URL into host and path
    std::smatch url_match_result;
    if (std::regex_match(url, url_match_result, url_regex)) {
        return { url_match_result[1].str(), url_match_result[2].str() };  // Returns the parsed host and path
    }
    return { "", "" };  // Returns empty strings if the URL does not match the expected format
}

bool download_and_save_file(const std::string& url, const std::string& output_filename) {
    ParsedURL parsed_url = parse_url(url);  // Parses the URL

    if (parsed_url.host.empty() || parsed_url.path.empty()) {
        std::cerr << "Invalid URL format: " << url << std::endl;  // Logs an error if the URL is invalid
        return false;
    }

    httplib::SSLClient cli(parsed_url.host.c_str());  // Initializes the HTTP client with the host

    auto res = cli.Get(parsed_url.path.c_str());  // Sends a GET request to the specified path

    if (res) {
        std::cout << "HTTP Status: " << res->status << std::endl;
        std::cout << "Response Length: " << res->body.size() << " bytes" << std::endl;

        if (res->status == 200) {
            std::ofstream image_file(output_filename, std::ios::out | std::ios::binary);  // Opens a file to save the downloaded content
            if (image_file.is_open()) {
                image_file.write(res->body.c_str(), res->body.size());  // Writes the response body to the file
                image_file.close();
                std::cout << "File written successfully: " << output_filename << std::endl;
            }
            else {
                std::cerr << "Failed to open the file for writing: " << output_filename << std::endl;
                return false;  // Returns false if the file could not be opened
            }
        }
        else {
            std::cerr << "HTTP request failed with status " << res->status << std::endl;
            return false;  // Returns false if the HTTP request failed
        }
    }
    else {
        std::cerr << "Failed to make the HTTP request." << std::endl;
        return false;  // Returns false if the HTTP request could not be made
    }

    return true;  // Returns true if the file was successfully downloaded and saved
}

void DownloadThread::operator()(CommonObjects& common) {
    while (!common.exit_flag) {  // Continues looping until the exit flag is set
        std::unique_lock<std::mutex> lock(common.mtx);  // Locks the mutex for thread-safe access
        common.cv.wait(lock, [&common] { return common.download_needed || common.exit_flag; });  // Waits until download is needed or exit is flagged

        if (common.exit_flag) break;  // Exits the loop if the exit flag is set

        if (common.m_window_shown == WindowShown::ALL_PRODUCTS) {
            httplib::SSLClient cli(_download_url);  // Initializes the HTTP client with the download URL
            auto res = cli.Get("/products?limit=50");  // Sends a GET request to fetch the products
            if (res && res->status == HTTP_STATUS_SUCCESS) {
                auto json_result = nlohmann::json::parse(res->body);  // Parses the JSON response
                common.products = json_result["products"].get<std::vector<Product>>();  // Populates the products vector

                for (int i = 0; i < common.products.size(); ++i) {
                    common.products[i].reviews = json_result["products"][i]["reviews"];  // Assigns the reviews to each product
                }

                if (!common.products.empty()) {
                    common.data_ready = true;  // Sets the data_ready flag if products were successfully retrieved
                }
            }
            else {
                std::cerr << "Failed to fetch products: " << (res ? res->status : -1) << std::endl;  // Logs an error if the request failed
            }
        }
        else if (common.m_window_shown == WindowShown::SPECIFIC_PRODUCT &&
            common.selected_product_index >= 0) {
            auto& rec = common.products[common.selected_product_index];  // References the selected product
            for (const auto& image_url : rec.images) {
                std::string filename = "image_" + std::to_string(common.selected_product_index) + "_" + std::to_string(&image_url - &rec.images[0]) + ".png";  // Constructs the filename for the image

                bool is_found_image = false;
                for (const auto& it : common.downloaded_images_paths[common.selected_product_index]) {
                    if (it == filename)
                        is_found_image = true;  // Checks if the image has already been downloaded
                }

                if (is_found_image)
                    continue;  // Skips downloading if the image is already found

                if (download_and_save_file(image_url, filename)) {  // Downloads and saves the image file
                    common.downloaded_images_paths[common.selected_product_index].push_back(filename);  // Adds the filename to the downloaded images map
                }
            }
        }

        common.download_needed = false;  // Resets the download_needed flag after processing
    }
}

void DownloadThread::SetUrl(std::string_view new_url) {
    _download_url = new_url;  // Sets the download URL for the thread
}
