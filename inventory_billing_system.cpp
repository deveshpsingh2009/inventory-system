/*
    Project: Inventory and Billing System
    Subject: Object-Oriented Programming Using C++ (CIC-211)
    Standard: C++11 or later

    Features:
    - Add products with unique IDs, names, prices and stock quantities
    - Display the product catalogue
    - Search for a product by ID
    - Prepare bills containing one or more products
    - Validate requested quantities against available stock
    - Reduce stock only after the bill is confirmed
    - Save and load product records using inventory_products.txt

    Inventory file format:
    product_id|price|quantity|product_name
*/

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

class Product {
private:
    int id;
    std::string name;
    double price;
    int quantity;

public:
    Product(int productId, const std::string& productName,
            double productPrice, int productQuantity)
        : id(productId),
          name(productName),
          price(productPrice),
          quantity(productQuantity) {
    }

    int getId() const { return id; }
    const std::string& getName() const { return name; }
    double getPrice() const { return price; }
    int getQuantity() const { return quantity; }

    void setQuantity(int newQuantity) {
        quantity = newQuantity;
    }
};

class InventorySystem {
private:
    struct BillItem {
        int productId;
        std::string productName;
        int quantity;
        double unitPrice;

        double lineAmount() const {
            return unitPrice * quantity;
        }
    };

    std::vector<Product> products;
    const std::string inventoryFile;

    int findProductIndex(int productId) const {
        for (std::size_t i = 0; i < products.size(); ++i) {
            if (products[i].getId() == productId) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    static bool isValidProductName(const std::string& name) {
        return !name.empty() &&
               name.find('|') == std::string::npos &&
               name.find('\n') == std::string::npos &&
               name.find('\r') == std::string::npos;
    }

    static bool parseInteger(const std::string& input, int& value) {
        std::istringstream stream(input);
        char extra;
        return (stream >> value) && !(stream >> extra);
    }

    static bool parseDouble(const std::string& input, double& value) {
        std::istringstream stream(input);
        char extra;
        if (!(stream >> value) || (stream >> extra)) {
            return false;
        }
        return std::isfinite(value);
    }

    static int readInteger(const std::string& prompt) {
        std::string input;
        int value;

        while (true) {
            std::cout << prompt;
            std::getline(std::cin, input);

            if (!std::cin) {
                std::cout << "\nInput ended. Exiting the program.\n";
                std::exit(0);
            }

            if (parseInteger(input, value)) {
                return value;
            }
            std::cout << "Invalid input. Please enter a whole number.\n";
        }
    }

    static double readDouble(const std::string& prompt) {
        std::string input;
        double value;

        while (true) {
            std::cout << prompt;
            std::getline(std::cin, input);

            if (!std::cin) {
                std::cout << "\nInput ended. Exiting the program.\n";
                std::exit(0);
            }

            if (parseDouble(input, value)) {
                return value;
            }
            std::cout << "Invalid input. Please enter a valid number.\n";
        }
    }

    static std::string readText(const std::string& prompt) {
        std::string value;

        while (true) {
            std::cout << prompt;
            std::getline(std::cin, value);

            if (!std::cin) {
                std::cout << "\nInput ended. Exiting the program.\n";
                std::exit(0);
            }

            if (!value.empty()) {
                return value;
            }
            std::cout << "This field cannot be empty.\n";
        }
    }

    static bool askYesNo(const std::string& prompt) {
        std::string answer;

        while (true) {
            std::cout << prompt << " (y/n): ";
            std::getline(std::cin, answer);

            if (!std::cin) {
                return false;
            }

            if (answer == "y" || answer == "Y" ||
                answer == "yes" || answer == "YES" ||
                answer == "Yes") {
                return true;
            }

            if (answer == "n" || answer == "N" ||
                answer == "no" || answer == "NO" ||
                answer == "No") {
                return false;
            }

            std::cout << "Please enter y or n.\n";
        }
    }

    bool saveInventoryToFile(const std::vector<Product>& data) const {
        std::ofstream file(inventoryFile.c_str());

        if (!file) {
            std::cout << "Error: Could not open " << inventoryFile
                      << " for saving.\n";
            return false;
        }

        file << std::setprecision(17);
        for (std::size_t i = 0; i < data.size(); ++i) {
            file << data[i].getId() << '|'
                 << data[i].getPrice() << '|'
                 << data[i].getQuantity() << '|'
                 << data[i].getName() << '\n';

            if (!file) {
                std::cout << "Error: Could not write inventory data.\n";
                return false;
            }
        }

        file.close();
        if (!file) {
            std::cout << "Error: Could not finish saving inventory data.\n";
            return false;
        }

        return true;
    }

    void loadInventory() {
        std::ifstream file(inventoryFile.c_str());

        if (!file) {
            std::cout << "No previous inventory file found. "
                      << "Starting with an empty inventory.\n";
            return;
        }

        std::string line;
        int loadedCount = 0;
        int skippedCount = 0;

        while (std::getline(file, line)) {
            if (line.empty()) {
                continue;
            }

            std::istringstream record(line);
            std::string idText;
            std::string priceText;
            std::string quantityText;
            std::string name;

            if (!std::getline(record, idText, '|') ||
                !std::getline(record, priceText, '|') ||
                !std::getline(record, quantityText, '|') ||
                !std::getline(record, name)) {
                ++skippedCount;
                continue;
            }

            int idValue;
            int quantityValue;
            double priceValue;

            if (!parseInteger(idText, idValue) ||
                !parseInteger(quantityText, quantityValue) ||
                !parseDouble(priceText, priceValue) ||
                idValue <= 0 || quantityValue < 0 ||
                priceValue <= 0.0 || !isValidProductName(name) ||
                findProductIndex(idValue) != -1) {
                ++skippedCount;
                continue;
            }

            products.push_back(
                Product(idValue, name, priceValue, quantityValue)
            );
            ++loadedCount;
        }

        std::cout << loadedCount << " product(s) loaded from "
                  << inventoryFile << ".\n";

        if (skippedCount > 0) {
            std::cout << "Warning: " << skippedCount
                      << " invalid or duplicate record(s) were skipped.\n";
        }
    }

    void addProduct() {
        std::cout << "\n===== ADD PRODUCT =====\n";

        const int id = readInteger("Enter product ID: ");
        if (id <= 0) {
            std::cout << "Product ID must be a positive number.\n";
            return;
        }

        if (findProductIndex(id) != -1) {
            std::cout << "This product ID already exists. "
                      << "Please use a unique ID.\n";
            return;
        }

        const std::string name = readText("Enter product name: ");
        if (!isValidProductName(name)) {
            std::cout << "Product name cannot contain the | character "
                      << "or a line break.\n";
            return;
        }

        const double price = readDouble("Enter unit price (Rs.): ");
        if (price <= 0.0) {
            std::cout << "Price must be greater than zero.\n";
            return;
        }

        const int quantity = readInteger("Enter available quantity: ");
        if (quantity < 0) {
            std::cout << "Quantity cannot be negative.\n";
            return;
        }

        std::vector<Product> updatedProducts = products;
        updatedProducts.push_back(Product(id, name, price, quantity));

        if (saveInventoryToFile(updatedProducts)) {
            products.swap(updatedProducts);
            std::cout << "Product added and inventory saved successfully.\n";
        } else {
            std::cout << "Product was not added because saving failed.\n";
        }
    }

    void displayProducts() const {
        std::cout << "\n===== PRODUCT CATALOGUE =====\n";

        if (products.empty()) {
            std::cout << "No products are available. Add products first.\n";
            return;
        }

        std::cout << std::left
                  << std::setw(12) << "Product ID"
                  << std::setw(25) << "Product Name"
                  << std::right
                  << std::setw(15) << "Unit Price"
                  << std::setw(15) << "Stock" << '\n';

        std::cout << std::string(67, '-') << '\n';

        for (std::size_t i = 0; i < products.size(); ++i) {
            std::cout << std::left
                      << std::setw(12) << products[i].getId()
                      << std::setw(25)
                      << products[i].getName().substr(0, 24)
                      << std::right
                      << std::setw(15) << std::fixed
                      << std::setprecision(2) << products[i].getPrice()
                      << std::setw(15) << products[i].getQuantity()
                      << '\n';
        }
    }

    void searchProduct() const {
        std::cout << "\n===== SEARCH PRODUCT =====\n";

        const int id = readInteger("Enter product ID to search: ");
        const int index = findProductIndex(id);

        if (index == -1) {
            std::cout << "Product with ID " << id << " was not found.\n";
            return;
        }

        const Product& product = products[static_cast<std::size_t>(index)];

        std::cout << "\nProduct found:\n";
        std::cout << "Product ID   : " << product.getId() << '\n';
        std::cout << "Product Name : " << product.getName() << '\n';
        std::cout << "Unit Price   : Rs. " << std::fixed
                  << std::setprecision(2) << product.getPrice() << '\n';
        std::cout << "Available    : " << product.getQuantity() << '\n';
    }

    void createBill() {
        std::cout << "\n===== CREATE CUSTOMER BILL =====\n";

        if (products.empty()) {
            std::cout << "Inventory is empty. Add products before billing.\n";
            return;
        }

        std::vector<BillItem> billItems;

        while (true) {
            const int id = readInteger(
                "Enter product ID (0 to finish adding items): "
            );

            if (id == 0) {
                break;
            }

            const int index = findProductIndex(id);
            if (index == -1) {
                std::cout << "Product not found. Please check the ID.\n";
                continue;
            }

            const Product& product =
                products[static_cast<std::size_t>(index)];

            int alreadySelected = 0;
            for (std::size_t i = 0; i < billItems.size(); ++i) {
                if (billItems[i].productId == id) {
                    alreadySelected += billItems[i].quantity;
                }
            }

            const int remainingStock =
                product.getQuantity() - alreadySelected;

            if (remainingStock <= 0) {
                std::cout << "No more units of this product are available "
                          << "for this bill.\n";
                continue;
            }

            const int requestedQuantity =
                readInteger("Enter quantity required: ");

            if (requestedQuantity <= 0) {
                std::cout << "Quantity must be greater than zero.\n";
                continue;
            }

            if (requestedQuantity > remainingStock) {
                std::cout << "Insufficient stock. Available for this bill: "
                          << remainingStock << '\n';
                continue;
            }

            BillItem item;
            item.productId = id;
            item.productName = product.getName();
            item.quantity = requestedQuantity;
            item.unitPrice = product.getPrice();
            billItems.push_back(item);

            std::cout << "Item added to bill.\n";

            if (!askYesNo("Do you want to add another product?")) {
                break;
            }
        }

        if (billItems.empty()) {
            std::cout << "No items were added. Bill cancelled.\n";
            return;
        }

        double subtotal = 0.0;

        std::cout << "\n================ CUSTOMER BILL ================\n";
        std::cout << std::left
                  << std::setw(24) << "Product"
                  << std::right
                  << std::setw(10) << "Quantity"
                  << std::setw(14) << "Unit Price"
                  << std::setw(15) << "Line Amount" << '\n';
        std::cout << std::string(63, '-') << '\n';

        for (std::size_t i = 0; i < billItems.size(); ++i) {
            const BillItem& item = billItems[i];
            subtotal += item.lineAmount();

            std::cout << std::left
                      << std::setw(24) << item.productName.substr(0, 23)
                      << std::right
                      << std::setw(10) << item.quantity
                      << std::setw(14) << std::fixed
                      << std::setprecision(2) << item.unitPrice
                      << std::setw(15) << item.lineAmount() << '\n';
        }

        std::cout << std::string(63, '-') << '\n';
        std::cout << std::right << std::setw(48) << "Subtotal: Rs. "
                  << std::fixed << std::setprecision(2) << subtotal << '\n';
        std::cout << std::setw(48) << "Final amount: Rs. "
                  << subtotal << '\n';
        std::cout << "================================================\n";

        if (!askYesNo("Confirm this bill and update inventory?")) {
            std::cout << "Bill cancelled. Inventory has not been changed.\n";
            return;
        }

        // Update a temporary copy first. The live inventory changes only
        // after the updated records have been saved successfully.
        std::vector<Product> updatedProducts = products;

        for (std::size_t i = 0; i < billItems.size(); ++i) {
            for (std::size_t j = 0; j < updatedProducts.size(); ++j) {
                if (updatedProducts[j].getId() == billItems[i].productId) {
                    const int newQuantity =
                        updatedProducts[j].getQuantity() -
                        billItems[i].quantity;
                    updatedProducts[j].setQuantity(newQuantity);
                    break;
                }
            }
        }

        if (saveInventoryToFile(updatedProducts)) {
            products.swap(updatedProducts);
            std::cout << "Bill confirmed. Inventory updated and saved.\n";
        } else {
            std::cout << "Could not save the updated inventory. "
                      << "The in-memory stock was not changed.\n";
        }
    }

    void showMenu() const {
        std::cout << "\n========== INVENTORY AND BILLING SYSTEM ==========\n";
        std::cout << "1. Add Product\n";
        std::cout << "2. Display All Products\n";
        std::cout << "3. Search Product by ID\n";
        std::cout << "4. Create Customer Bill\n";
        std::cout << "5. Exit\n";
        std::cout << "===================================================\n";
    }

public:
    InventorySystem()
        : inventoryFile("inventory_products.txt") {
        loadInventory();
    }

    void run() {
        while (true) {
            showMenu();
            const int choice = readInteger("Enter your choice: ");

            switch (choice) {
                case 1:
                    addProduct();
                    break;
                case 2:
                    displayProducts();
                    break;
                case 3:
                    searchProduct();
                    break;
                case 4:
                    createBill();
                    break;
                case 5:
                    std::cout << "Thank you for using the Inventory "
                              << "and Billing System.\n";
                    return;
                default:
                    std::cout << "Invalid choice. Please select 1 to 5.\n";
            }
        }
    }
};

int main() {
    InventorySystem system;
    system.run();
    return 0;
}
