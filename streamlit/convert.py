import base64
import os

def image_to_base64_data_url(image_path):
    """
    Converts an image file to a Base64 data URL.
    """
    try:
        # Guess the MIME type based on the file extension
        ext = os.path.splitext(image_path)[1].lower()
        mime_type = {
            '.jpg': 'image/jpeg',
            '.jpeg': 'image/jpeg',
            '.png': 'image/png',
            '.gif': 'image/gif',
            '.svg': 'image/svg+xml',
            '.webp': 'image/webp',
            '.bmp': 'image/bmp'

        }.get(ext)

        if not mime_type:
            return "Error: Unsupported image file type or unknown extension."

        with open(image_path, "rb") as image_file:
            encoded_string = base64.b64encode(image_file.read()).decode('utf-8')

        return f"data:{mime_type};base64,{encoded_string}"

    except FileNotFoundError:
        return f"Error: Image file not found at '{image_path}'."
    except Exception as e:
        return f"An unexpected error occurred: {e}"

# -------------------------------------------------------------------------
# IMPORTANT: Replace 'your_image_filename.jpg' with the actual path to your image.
# If the image is in the same directory as this script, just its name is fine.
# Otherwise, provide the full or relative path.
# Example paths:
# image_path_to_convert = 'mona_lisa.jpg'
# image_path_to_convert = 'assets/images/my_background.png'
# image_path_to_convert = '/path/to/your/image.jpeg'
# -------------------------------------------------------------------------

image_path_to_convert = 'C:/Users/Sujal/OneDrive/Desktop/New folder/Emertxe projects/staegnography/data/encode_input/beautiful.bmp'
# --- Generate the Base64 string ---
base64_data_url = image_to_base64_data_url(image_path_to_convert)

if "Error:" not in base64_data_url:
    print("\nSuccessfully encoded the image!")
    print("Copy the ENTIRE string below (it will be very long) and paste it into your Streamlit app's CSS:")
    print("-" * 70)
    print(base64_data_url)
    print("-" * 70)
else:
    print(f"\n{base64_data_url}")