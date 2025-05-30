import streamlit as st
import os
import sys
import time

# Use wide layout for full-width columns
st.set_page_config(page_title=" Finn Mcmissile- LSB Image stegonographer ", layout="wide")
st.subheader("The devil is in the details")
# Inject CSS
st.markdown("""
<style>
/* Constrain dropzone width */
div[data-testid="stFileUploaderDropzone"] {
  max-width: 300px;
}

/* Apply min-height ONLY to bordered containers within our custom 'input-panel-container' wrapper */
.input-panel-container div[data-testid="stVerticalBlock"] > div[data-testid="stVerticalBlockBorderWrapper"] > div[data-testid="stVerticalBlock"] {
    min-height: 300px; /* Adjust this value for Encode Input & Decode Input height as needed */
    display: flex;
    flex-direction: column;
    /* justify-content: space-between; */ /* Optional: if you want internal elements to spread out */
}

/* Optional: Ensure consistent bottom margin for subheaders within all bordered containers */
div[data-testid="stVerticalBlock"] > div[data-testid="stVerticalBlockBorderWrapper"] > div[data-testid="stVerticalBlock"] h3 { /* Targets st.subheader */
    margin-bottom: 1rem;
}

</style>
""", unsafe_allow_html=True)

# Add parent directory to path for steganography_engine
# This assumes steganography_engine.py is in the parent directory of this app.py file
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
try:
    import steganography_engine
except ImportError as e:
    st.error(f"Failed to import steganography_engine: {e}. Ensure steganography_engine.py is in the correct path (e.g., parent directory).")
    st.stop()

# Create dirs for uploads/outputs
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
UPLOAD_DIR = os.path.join(BASE_DIR, "uploads")
OUTPUT_DIR = os.path.join(BASE_DIR, "outputs")
os.makedirs(UPLOAD_DIR, exist_ok=True)
os.makedirs(OUTPUT_DIR, exist_ok=True)

# Helper for unique filenames
def get_unique_filename(directory, base_filename):
    name, ext = os.path.splitext(base_filename)
    # Ensure extension starts with a dot if it's not empty
    if ext and not ext.startswith('.'):
        ext = '.' + ext
    elif not ext and name.endswith('.'): # Handle cases like "file."
        name = name[:-1]
        ext = '.bin' # Default extension if none provided meaningfully
    elif not ext: # Default extension if truly no extension
        ext = '.bin'
    # Sanitize base_filename
    name = "".join(c if c.isalnum() or c in (' ', '.', '_', '-') else '_' for c in name)
    return os.path.join(directory, f"{name}_{int(time.time() * 1000)}{ext}")


# Page Title
st.title("🖼️ LSB Image Steganography 🤫")

# Session state initialization
if 'decode_file' not in st.session_state:
    st.session_state['decode_file'] = None
if 'encoded_files_to_display' not in st.session_state:
    st.session_state['encoded_files_to_display'] = []
if 'decoded_files_to_display' not in st.session_state:
    st.session_state['decoded_files_to_display'] = []

# Layout columns with gap between panels
g1, enc_col, g2, dec_col, g3 = st.columns([0.5, 5, 0.5, 5, 0.5])

# --- Encode Section ---
with enc_col:
    st.header("🔒 Encode")

    # Add wrapper div for Encode Input
    st.markdown('<div class="input-panel-container">', unsafe_allow_html=True)
    with st.container(border=True):
        st.subheader("Encode Input")
        c1, c2 = st.columns(2)
        with c1:
            src_image = st.file_uploader(
                "Source Image (BMP)",
                type=["bmp"],
                key="enc_src",
                help="Select the BMP image you want to hide data in."
            )
        with c2:
            secret_file = st.file_uploader(
                "Secret File",
                key="enc_secret",
                help="Select any file that you want to hide."
            )
        magic_enc = st.text_input(
            "Magic String (Password)",
            type="password",
            key="ms_encode",
            help="Enter a password to secure your hidden file."
        )
        if st.button("✨ Encode", key="enc_button", use_container_width=True):
            if src_image and secret_file and magic_enc:
                src_path = get_unique_filename(UPLOAD_DIR, src_image.name)
                with open(src_path, 'wb') as f: f.write(src_image.getbuffer())

                secret_path = get_unique_filename(UPLOAD_DIR, secret_file.name)
                with open(secret_path, 'wb') as f: f.write(secret_file.getbuffer())

                base_stego_name, _ = os.path.splitext(src_image.name)
                # Ensure the stego image name is also BMP
                stego_image_name_with_ext = f"stego_{base_stego_name}.bmp"
                stego_path = get_unique_filename(OUTPUT_DIR, stego_image_name_with_ext)

                with st.spinner("Encoding... This may take a moment."):
                    try:
                        res = steganography_engine.encode(src_path, secret_path, stego_path, magic_enc)
                        if res.success:
                            st.success(f"Successfully encoded and saved: {os.path.basename(res.output_path)}")
                            st.session_state['encoded_files_to_display'] = sorted([f for f in os.listdir(OUTPUT_DIR) if f.startswith("stego_")])
                        else:
                            st.error(f"Encoding failed: {res.message}")
                    except Exception as e:
                        st.error(f"An unexpected error occurred during encoding: {e}")
                    finally:
                        if os.path.exists(src_path): os.remove(src_path)
                        if os.path.exists(secret_path): os.remove(secret_path)
            else:
                st.warning("Please provide a source image, a secret file, and a password to encode.")
        st.markdown("<br>", unsafe_allow_html=True) # Adds a bit of space for aesthetics
    st.markdown('</div>', unsafe_allow_html=True) # Close wrapper div for Encode Input

    st.markdown("---") # Visual separator

    with st.container(border=True): # Encode Output (no specific min-height from CSS rule)
        st.subheader("Encode Output")
        if st.button("🔄 Refresh Encode Output", key="refresh_enc", use_container_width=True):
            st.session_state['encoded_files_to_display'] = sorted([f for f in os.listdir(OUTPUT_DIR) if f.startswith("stego_")])
            st.rerun() # Rerun to refresh the list immediately

        # Initialize if empty, to avoid error on first load or after clearing outputs
        if not st.session_state.get('encoded_files_to_display'):
             st.session_state['encoded_files_to_display'] = sorted([f for f in os.listdir(OUTPUT_DIR) if f.startswith("stego_")])

        if st.session_state.get('encoded_files_to_display'):
            for fn in st.session_state['encoded_files_to_display']:
                p = os.path.join(OUTPUT_DIR, fn)
                if os.path.exists(p):
                    coln, cold, colu = st.columns([4, 1, 2])
                    coln.write(fn)
                    with open(p, 'rb') as f_download:
                        cold.download_button("📥", f_download, file_name=fn, mime="image/bmp", key=f"dl_enc_{fn}", help="Download stego image")
                    if colu.button("→ Use in Decode", key=f"use_{fn}", help="Send this file to the decode panel"):
                        st.session_state['decode_file'] = p
                        st.rerun()
        else:
            st.info("No stego files found. Encode a file and then refresh if needed.")
        st.markdown("<br>", unsafe_allow_html=True)

# --- Decode Section ---
with dec_col:
    st.header("🔓 Decode")
    
    upload = None # To track if a new file was uploaded in this cycle
    stego_path_decode_input = None # Path for the file to be decoded

    # Add wrapper div for Decode Input
    st.markdown('<div class="input-panel-container">', unsafe_allow_html=True)
    with st.container(border=True):
        st.subheader("Decode Input")
        if st.session_state.get('decode_file'):
            stego_path_decode_input = st.session_state['decode_file']
            st.success(f"Selected for Decode: **{os.path.basename(stego_path_decode_input)}**")
            if st.button("Clear selected stego image", key="clear_decode_selection", use_container_width=True):
                st.session_state['decode_file'] = None
                stego_path_decode_input = None 
                st.rerun()
        else:
            upload = st.file_uploader(
                "Stego Image (BMP)",
                type=["bmp"],
                key="dec_src", # Unique key
                help="Upload the BMP image containing hidden data."
            )
            if upload:
                stego_path_decode_input = get_unique_filename(UPLOAD_DIR, upload.name)
                with open(stego_path_decode_input, 'wb') as f: f.write(upload.getbuffer())
            elif not st.session_state.get('decode_file'): # Ensure path is None if no selection and no upload
                stego_path_decode_input = None

        magic_dec = st.text_input(
            "Magic String (Password)",
            type="password",
            key="ms_decode", # Unique key
            help="Enter the password used during encoding."
        )
        if st.button("🔍 Decode", key="dec_button", use_container_width=True): # Unique key
            if stego_path_decode_input and magic_dec:
                original_stego_name = os.path.basename(stego_path_decode_input)
                base_name_for_output = original_stego_name
                if base_name_for_output.startswith("stego_"):
                    temp_name = base_name_for_output.replace("stego_", "", 1)
                    name_part, ext_part = os.path.splitext(temp_name)
                    parts = name_part.split('_')
                    if len(parts) > 1 and parts[-1].isdigit() and len(parts[-1]) > 10: # Heuristic for timestamp
                         base_name_for_output = "_".join(parts[:-1]) + ext_part
                    else:
                         base_name_for_output = temp_name
                else:
                    base_name_for_output, _ = os.path.splitext(base_name_for_output)
                
                out_base_path_prefix = os.path.join(OUTPUT_DIR, f"decoded_{base_name_for_output}")

                with st.spinner("Decoding... This may take a moment."):
                    try:
                        res = steganography_engine.decode(stego_path_decode_input, out_base_path_prefix, magic_dec)
                        if res.success and res.output_path and os.path.exists(res.output_path):
                            st.success(f"Successfully decoded and saved: {os.path.basename(res.output_path)}")
                            st.session_state['decoded_files_to_display'] = sorted([f for f in os.listdir(OUTPUT_DIR) if not f.startswith("stego_") and not f.endswith("tmp")])
                        else:
                            st.error(res.message or "Decoding failed and no output file was generated.")
                    except Exception as e:
                        st.error(f"An unexpected error occurred during decoding: {e}")
                    finally:
                        # Clean up temporary uploaded decode file only if 'upload' object exists and path is in UPLOAD_DIR
                        if upload and stego_path_decode_input and os.path.exists(stego_path_decode_input):
                             if UPLOAD_DIR in stego_path_decode_input:
                                os.remove(stego_path_decode_input)
            elif not stego_path_decode_input:
                st.warning("Please select or upload a stego image to decode.")
            else: # Only magic string missing
                st.warning("Please provide the password to decode.")
        st.markdown("<br>", unsafe_allow_html=True)
    st.markdown('</div>', unsafe_allow_html=True) # Close wrapper div for Decode Input

    st.markdown("---") # Visual separator

    with st.container(border=True): # Decode Output (no specific min-height from CSS rule)
        st.subheader("Decode Output")
        if st.button("🔄 Refresh Decode Output", key="refresh_dec", use_container_width=True): # Unique key
            st.session_state['decoded_files_to_display'] = sorted([f for f in os.listdir(OUTPUT_DIR) if not f.startswith("stego_") and not f.endswith("tmp")])
            st.rerun()

        if not st.session_state.get('decoded_files_to_display'):
            st.session_state['decoded_files_to_display'] = sorted([f for f in os.listdir(OUTPUT_DIR) if not f.startswith("stego_") and not f.endswith("tmp")])

        if st.session_state.get('decoded_files_to_display'):
            for fn in st.session_state['decoded_files_to_display']:
                p = os.path.join(OUTPUT_DIR, fn)
                if os.path.exists(p):
                    c1, c2 = st.columns([4, 1])
                    c1.write(fn)
                    with open(p, 'rb') as f_download:
                        c2.download_button("📥", f_download, file_name=fn, key=f"dl_dec_{fn}", help="Download decoded file")
        else:
            st.info("No decoded files found. Decode a file and then refresh if needed.")
        st.markdown("<br>", unsafe_allow_html=True)
