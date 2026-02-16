#include "Program.hpp"
#include <CLI/CLI.hpp>
#include <iostream>

int main(int argc, char **argv)
{
    CLI::App app{"Ray Tracing Demo"};

    std::string sceneFilePath = "scenes/default.json";
    app.add_option("-s,--scene", sceneFilePath, "Path to the scene JSON file");

    CLI11_PARSE(app, argc, argv);

    try {
        Program p(sceneFilePath);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
