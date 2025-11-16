#include <iostream>
#include "gui/Application.h"
#include "core/hash.h"

int main() {
    try {
        Application app;
        if (app.initialize()) {
            app.run();
        }
    } catch (const std::exception& e) {
        std::cerr << "Помилка: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}
