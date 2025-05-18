# Convert the list string back to a CMake list
string(REPLACE " " ";" SHADER_FILES "${SHADER_FILES}")

# Filter to only include .frag files if you only want fragment shaders
set(FRAG_SHADER_FILES "")
foreach(shader_file IN LISTS SHADER_FILES)
    if(shader_file MATCHES ".*\\.frag$")
        list(APPEND FRAG_SHADER_FILES "${shader_file}")
    endif()
endforeach()

# Make sure the output directory exists
get_filename_component(OUTPUT_DIR "${SHADERS_HEADER}" DIRECTORY)
file(MAKE_DIRECTORY "${OUTPUT_DIR}")

# Start writing the header file with a guard
file(WRITE "${SHADERS_HEADER}" "#ifndef SHADERS_H\n#define SHADERS_H\n\n#include <string>\n#include <unordered_map>\n\nnamespace Shaders {\n\n")

# Process each shader file
foreach(shader_file IN LISTS SHADER_FILES)
    # Get filename without path and extension
    get_filename_component(shader_name "${shader_file}" NAME_WE)
    
    # Read shader content
    file(READ "${shader_file}" shader_content)
    
    # Escape special characters
    string(REPLACE "\\" "\\\\" shader_content "${shader_content}")
    string(REPLACE "\"" "\\\"" shader_content "${shader_content}")
    string(REPLACE "\n" "\\n\"\n\"" shader_content "${shader_content}")
    
    # Write to header as raw string
    file(APPEND "${SHADERS_HEADER}" "const std::string ${shader_name}Shader = \"${shader_content}\";\n\n")
endforeach()

# Add a map for easy lookup by name
file(APPEND "${SHADERS_HEADER}" "const std::unordered_map<std::string, std::string> shaderMap = {\n")

foreach(shader_file IN LISTS SHADER_FILES)
    get_filename_component(shader_name "${shader_file}" NAME_WE)
    file(APPEND "${SHADERS_HEADER}" "    {\"${shader_name}\", ${shader_name}Shader},\n")
endforeach()

file(APPEND "${SHADERS_HEADER}" "};\n\n} // namespace Shaders\n\n#endif // SHADERS_H\n")