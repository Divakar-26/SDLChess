import os

# Define the directory where the board textures are stored
board_texture_folder = "boardTexture"

# Loop through all files in the boardTexture folder
for filename in os.listdir(board_texture_folder):
    # Check if the file ends with '_200.png'
    if filename.endswith("_200.png"):
        # Create the new filename by removing '_200' from the name
        new_filename = filename.replace("_200", "")  # Remove the '_200' part

        # Construct the full file paths
        old_file_path = os.path.join(board_texture_folder, filename)
        new_file_path = os.path.join(board_texture_folder, new_filename)

        # Check if the new file already exists
        if os.path.exists(new_file_path):
            # If the new file exists, remove it to replace with the new one
            os.remove(new_file_path)
            print(f"Existing file {new_filename} found and removed.")

        # Rename (move) the old file to the new file name
        os.rename(old_file_path, new_file_path)
        print(f"File renamed: {filename} -> {new_filename}")
