# Convert the list string back to a CMake list
string(REPLACE " " ";" SHADER_FILES "${SHADER_FILES}")

# Make sure the output directory exists
get_filename_component(OUTPUT_DIR "${SHADERS_HEADER}" DIRECTORY)
file(MAKE_DIRECTORY "${OUTPUT_DIR}")

# Start writing the header file with a guard
file(WRITE "${SHADERS_HEADER}" "#pragma once\n\n#include <string>\n#include <unordered_map>\n\nnamespace Supernova::Editor {\n\n")

# Process each shader file
foreach(shader_file IN LISTS SHADER_FILES)
    # Get filename with extension
    get_filename_component(shader_name "${shader_file}" NAME)

    # Create a valid C++ variable name by replacing dots with underscores
    string(REPLACE "." "_" cpp_var_name "${shader_name}")
    
    # Read shader content
    file(READ "${shader_file}" shader_content)
    
    # Escape special characters
    string(REPLACE "\\" "\\\\" shader_content "${shader_content}")
    string(REPLACE "\"" "\\\"" shader_content "${shader_content}")
    string(REPLACE "\n" "\\n\"\n\"" shader_content "${shader_content}")
    
    # Write to header as raw string
    file(APPEND "${SHADERS_HEADER}" "const std::string ${cpp_var_name}_shader = \"${shader_content}\";\n\n")
endforeach()

# Add a map for easy lookup by name
file(APPEND "${SHADERS_HEADER}" "const std::unordered_map<std::string, std::string> shaderMap = {\n")

foreach(shader_file IN LISTS SHADER_FILES)
    get_filename_component(shader_name "${shader_file}" NAME)
    string(REPLACE "." "_" cpp_var_name "${shader_name}")
    file(APPEND "${SHADERS_HEADER}" "    {\"${shader_name}\", ${cpp_var_name}_shader},\n")
endforeach()

file(APPEND "${SHADERS_HEADER}" "};\n\n}\n")
