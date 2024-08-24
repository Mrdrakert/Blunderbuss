import os

def parse_txt_files(folder_path):
    # Iterate through all files in the given folder
    for filename in os.listdir(folder_path):
        if filename.endswith(".txt"):
            file_path = os.path.join(folder_path, filename)
            with open(file_path, 'r') as file:
                # Read all lines from the file
                lines = file.readlines()

                if len(lines) != 64:
                    print(f"Warning: File {filename} does not have exactly 64 lines. Skipping this file.")
                    continue
                
                # Remove any extra whitespace characters (like \n)
                lines = [line.strip() for line in lines]
                
                # Group lines in sets of 8
                grouped_lines = [", ".join(lines[i:i+8]) for i in range(0, 64, 8)]
                
                # Format the lines as uint64_t array
                formatted_lines = ",\n    ".join(grouped_lines)
                cpp_code = f"uint64_t array[64] = {{\n    {formatted_lines}\n}};\n"

                # Define output C++ file name
                output_filename = filename.replace(".txt", ".cpp")
                output_file_path = os.path.join(folder_path, output_filename)

                # Write the C++ formatted code to a new .cpp file
                with open(output_file_path, 'w') as cpp_file:
                    cpp_file.write(cpp_code)
                print(f"Processed {filename} -> {output_filename}")

# Set the path to the folder containing the .txt files
folder_path = "."

# Parse the .txt files and generate corresponding .cpp files
parse_txt_files(folder_path)
