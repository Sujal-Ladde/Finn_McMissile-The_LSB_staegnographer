import streamlit as st
import os
import sys
import time # For unique filenames

# Add the parent directory to sys.path to find the steganography_engine module
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
try:
    import steganography_engine
except ImportError as e:
    st.error(f"Failed to import steganography_engine: {e}. "
             "Make sure you have built the module using 'python setup.py build_ext --inplace' "
             "in the project root directory.")
    st.stop()


# Create directories for uploads and outputs if they don't exist
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
UPLOAD_DIR = os.path.join(BASE_DIR, "uploads")
OUTPUT_DIR = os.path.join(BASE_DIR, "outputs")
os.makedirs(UPLOAD_DIR, exist_ok=True)
os.makedirs(OUTPUT_DIR, exist_ok=True)

def get_unique_filename(directory, base_filename):
    """Generates a unique filename by appending a timestamp if necessary."""
    name, ext = os.path.splitext(base_filename)
    timestamp = int(time.time() * 1000) # Milliseconds for uniqueness
    filename = f"{name}_{timestamp}{ext}"
    return os.path.join(directory, filename)


st.title("🖼️ LSB Image Steganography 🤫")

mode = st.sidebar.selectbox("Choose mode:", ("Encode", "Decode"))

st.sidebar.markdown("---")
st.sidebar.markdown(
    """
    **How to use:**
    1.  Select **Encode** or **Decode** mode.
    2.  Upload the required files.
    3.  Enter a **Magic String** (password).
    4.  Click the button to process.
    5.  Download your resulting file.
    """
)


if mode == "Encode":
    st.header("🔒 Encode Secret into Image")

    src_image_file = st.file_uploader("1. Upload Source Image (BMP)", type=["bmp"])
    secret_file = st.file_uploader("2. Upload Secret File (e.g., .txt, .jpg)")
    magic_string_encode = st.text_input("3. Enter Magic String (Password):", type="password", key="ms_encode")

    if st.button("✨ Encode Image"):
        if src_image_file and secret_file and magic_string_encode:
            # Save uploaded files to temporary paths
            src_image_path = get_unique_filename(UPLOAD_DIR, src_image_file.name)
            with open(src_image_path, "wb") as f:
                f.write(src_image_file.getbuffer())

            secret_file_path = get_unique_filename(UPLOAD_DIR, secret_file.name)
            with open(secret_file_path, "wb") as f:
                f.write(secret_file.getbuffer())

            stego_image_basename = "stego_" + os.path.basename(src_image_path)
            stego_image_path = os.path.join(OUTPUT_DIR, stego_image_basename)
            # Ensure stego_image_path is unique if multiple runs occur
            stego_image_path = get_unique_filename(OUTPUT_DIR, stego_image_basename)


            with st.spinner("Encoding in progress..."):
                try:
                    result = steganography_engine.encode(
                        src_image_path,
                        secret_file_path,
                        stego_image_path,
                        magic_string_encode
                    )

                    if result.success:
                        st.success(f"✅ Encoding successful! Output: {os.path.basename(result.output_path)}")
                        with open(result.output_path, "rb") as file_to_download:
                            st.download_button(
                                label="📥 Download Stego Image",
                                data=file_to_download,
                                file_name=os.path.basename(result.output_path),
                                mime="image/bmp"
                            )
                    else:
                        st.error(f"❌ Encoding failed: {result.message}")
                except Exception as e:
                    st.error(f"An unexpected error occurred during encoding: {e}")
                finally:
                    # Clean up uploaded files
                    if os.path.exists(src_image_path):
                        os.remove(src_image_path)
                    if os.path.exists(secret_file_path):
                        os.remove(secret_file_path)
        else:
            st.warning("⚠️ Please upload all files and enter the magic string.")

elif mode == "Decode":
    st.header("🔓 Decode Secret from Image")

    stego_image_file_decode = st.file_uploader("1. Upload Stego Image (BMP)", type=["bmp"])
    magic_string_decode = st.text_input("2. Enter Magic String (Password):", type="password", key="ms_decode")

    if st.button("🔍 Decode Image"):
        if stego_image_file_decode and magic_string_decode:
            stego_image_path_decode = get_unique_filename(UPLOAD_DIR, stego_image_file_decode.name)
            with open(stego_image_path_decode, "wb") as f:
                f.write(stego_image_file_decode.getbuffer())

            # Base name for the output, extension will be added by C++ code
            output_secret_base_name = "decoded_secret" 
            output_secret_base_path = os.path.join(OUTPUT_DIR, output_secret_base_name)
            # The C++ binding will append the correct extension.
            # We use a unique base to avoid overwrites during the session.
            output_secret_base_path = get_unique_filename(OUTPUT_DIR, output_secret_base_name + ".tmp")[:-4] # remove .tmp

            with st.spinner("Decoding in progress..."):
                try:
                    result = steganography_engine.decode(
                        stego_image_path_decode,
                        output_secret_base_path, # Base path for output
                        magic_string_decode
                    )

                    if result.success and result.output_path and os.path.exists(result.output_path):
                        st.success(f"✅ Decoding successful! Output: {os.path.basename(result.output_path)}")
                        with open(result.output_path, "rb") as file_to_download:
                            st.download_button(
                                label="📥 Download Decoded File",
                                data=file_to_download,
                                file_name=os.path.basename(result.output_path)
                                # Mime type is unknown until decoded, so let browser guess or use generic
                            )
                    elif result.success and not result.output_path:
                         st.info("ℹ️ Decoding reported success, but no output path was returned or file does not exist. This might happen if the secret data size was 0.")
                    else:
                        st.error(f"❌ Decoding failed: {result.message}")

                except Exception as e:
                    st.error(f"An unexpected error occurred during decoding: {e}")
                finally:
                     if os.path.exists(stego_image_path_decode):
                        os.remove(stego_image_path_decode)
        else:
            st.warning("⚠️ Please upload the stego image and enter the magic string.")

st.sidebar.markdown("---")
st.sidebar.info("This app uses LSB steganography to hide data within BMP images.")