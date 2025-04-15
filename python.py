from PIL import Image
import cairosvg
import os

# Function to convert SVG to PNG using cairosvg
def convert_svg_to_png(svg_path, png_path):
    cairosvg.svg2png(url=svg_path, write_to=png_path)

# Function to combine images into a grid of 2 rows and 4 columns
def combine_images(image_paths, output_path):
    # Open all images
    images = [Image.open(path) for path in image_paths]

    # Get the width and height of the first image (assumes all images are the same size)
    img_width, img_height = images[0].size

    # Create a new blank image with the required size (2 rows, 4 columns)
    combined_width = img_width * 4
    combined_height = img_height * 2
    combined_image = Image.new('RGB', (combined_width, combined_height))

    # Paste images into the combined image
    for i, img in enumerate(images):
        row = i // 4
        col = i % 4
        x_offset = col * img_width
        y_offset = row * img_height
        combined_image.paste(img, (x_offset, y_offset))

    # Save the final combined image
    combined_image.save(output_path)

def main():
    # Paths to your 8 SVG files
    svg_files = [
        "svg1.svg", "svg2.svg", "svg3.svg", "svg4.svg", 
        "svg5.svg", "svg6.svg", "svg7.svg", "svg8.svg"
    ]
    
    # Convert each SVG to PNG
    png_files = []
    for i, svg_file in enumerate(svg_files):
        png_file = f"temp_{i}.png"
        convert_svg_to_png(svg_file, png_file)
        png_files.append(png_file)

    # Combine the PNGs into a single image
    output_path = "combined_image.png"
    combine_images(png_files, output_path)

    # Clean up temporary PNG files
    for png_file in png_files:
        os.remove(png_file)

    print(f"Combined image saved as {output_path}")

if __name__ == "__main__":
    main()
