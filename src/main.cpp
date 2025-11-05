/*
 * Bolt Package Manager (bolt-pm)
 * Main C++ Source File
 *
 * This tool manages the 'bolt.toml' manifest and calls
 * the 'bolt-compiler' to build projects.
 *
 * --- How to Compile This Tool ---
 * This project is built with CMake.
 * See CMakeLists.txt and README.md for instructions.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem> // C++17
#include <cstdlib>    // For system()

// This include will work because we'll tell CMake
// to look for headers inside the 'src' directory.
#include "toml.hpp"

// --- TOML Parsing ---
// We are now using the header-only 'toml.hpp' library.

namespace fs = std::filesystem;

const std::string MANIFEST_FILE = "bolt.toml";
const std::string COMPILER_NAME = "bolt-compiler"; // Our hypothetical compiler

// --- Real TOML Functions ---

// Creates a new bolt.toml file
void initialize_manifest() {
    if (fs::exists(MANIFEST_FILE)) {
        std::cout << "ℹ️ " << MANIFEST_FILE << " already exists." << std::endl;
        return;
    }

    // Create the TOML structure in memory
    toml::table tbl = toml::table{
        { "package", toml::table{
            { "name", "new-bolt-project" },
            { "version", "0.1.0" },
            { "entrypoint", "main.bolt" }
        }},
        { "dependencies", toml::table{} }
    };

    // Write the TOML table to the file
    std::ofstream manifest(MANIFEST_FILE);
    manifest << tbl;
    manifest.close();

    std::cout << "✨ Initialized new Bolt project in " << MANIFEST_FILE << std::endl;

    // Create the entrypoint file
    std::string entrypoint = "main.bolt";
    if (!fs::exists(entrypoint)) {
        std::ofstream entry(entrypoint);
        entry << "// Main entrypoint: " << entrypoint << "\n\nint main() {\n    \n    return 0;\n}\n";
        entry.close();
        std::cout << "✏️ Created entrypoint file: " << entrypoint << std::endl;
    }
}

// Adds a dependency to bolt.toml
void install_package(const std::string& package_name) {
    if (!fs::exists(MANIFEST_FILE)) {
        std::cerr << "❌ No " << MANIFEST_FILE << " found. Run 'bolt-pm new' first." << std::endl;
        return;
    }

    try {
        // Parse the existing file
        toml::table tbl = toml::parse_file(MANIFEST_FILE);

        // Get the dependencies table, or create it if it doesn't exist
        if (!tbl.contains("dependencies")) {
            tbl.insert("dependencies", toml::table{});
        }
        toml::table* deps = tbl["dependencies"].as_table();

        // Add or update the package
        deps->insert_or_assign(package_name, "1.0.0"); // Using a default version

        // Write the changes back to the file
        std::ofstream manifest(MANIFEST_FILE);
        manifest << tbl;
        manifest.close();

        std::cout << "✅ Added '" << package_name << " = \"1.0.0\"' to " << MANIFEST_FILE << "." << std::endl;
        std::cout << "Run 'bolt-pm build' to compile." << std::endl;

    } catch (const toml::parse_error& err) {
        std::cerr << "❌ Error parsing " << MANIFEST_FILE << ":\n" << err << std::endl;
    }
}

// Reads the manifest and calls the compiler
void build_project() {
    if (!fs::exists(MANIFEST_FILE)) {
        std::cerr << "❌ No " << MANIFEST_FILE << " found. Cannot build." << std::endl;
        return;
    }

    std::string entrypoint = "main.bolt";
    std::string output_name = "my-app";
    std::string dep_flags = "";

    try {
        toml::table tbl = toml::parse_file(MANIFEST_FILE);

        // Read package info, providing defaults if keys are missing
        entrypoint = tbl["package"]["entrypoint"].value_or("main.bolt");
        output_name = tbl["package"]["name"].value_or("my-app");

        // Build dependency flags
        if (auto deps = tbl["dependencies"].as_table()) {
            for (auto&& [key, val] : *deps) {
                // key.str() gives the package name (e.g., "fmt")
                dep_flags += " -l" + std::string(key.str()); 
            }
        }

    } catch (const toml::parse_error& err) {
        std::cerr << "❌ Error parsing " << MANIFEST_FILE << ":\n" << err << std::endl;
        return;
    }


    std::cout << "Building project '" << output_name << "' from " << entrypoint << "..." << std::endl;

    // Construct the compiler command
    std::string compiler_cmd = COMPILER_NAME;
    compiler_cmd += " " + entrypoint;
    compiler_cmd += " -o " + output_name;
    compiler_cmd += dep_flags; // Add all dependency flags

    std::cout << "Compiler command: " << compiler_cmd << std::endl;

    // Call the compiler using system()
    // A more robust solution would use <process> (C++26) or a library.
    int ret = std::system(compiler_cmd.c_str());

    if (ret == 0) {
        std::cout << "✅ Build successful! (Output: " << output_name << ")" << std::endl;
    } else {
        std::cerr << "❌ Build Failed. Make sure '" << COMPILER_NAME << "' is in your PATH." << std::endl;
    }
}

// --- Main CLI ---

void print_help() {
    std::cout << "Bolt Package Manager (bolt-pm)\n\n";
    std::cout << "Usage: bolt-pm <command>\n\n";
    std::cout << "Commands:\n";
    std::cout << "  new          Initializes a new project by creating bolt.toml\n";
    std::cout << "  install <pkg>  Adds a package to dependencies\n";
    std::cout << "  build        Compiles the project\n";
    std::cout << "  help         Show this help message\n";
}

int main(int argc, char* argv[]) {
    // Basic argument parsing
    if (argc < 2) {
        print_help();
        return 1;
    }

    std::string command = argv[1];

    if (command == "new") {
        initialize_manifest();
    } else if (command == "install") {
        if (argc < 3) {
            std::cerr << "❌ Error: 'install' requires a package name." << std::endl;
            return 1;
        }
        std::string package_name = argv[2];
        install_package(package_name);
    } else if (command == "build") {
        build_project();
    } else if (command == "help") {
        print_help();
    } else {
        std::cerr << "❌ Unknown command: " << command << std::endl;
        print_help();
        return 1;
    }

    return 0;
}