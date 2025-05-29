#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // For automatic type conversion (e.g., std::string)
#include "common.h"
#include "encode.h"
#include "decode.h"
#include <string>
#include <vector>
#include <cstdio>  // For snprintf
#include <cstring> // For strncpy, strcat, etc.

namespace py = pybind11;

// Helper to manage EncodeInfo and file operations consistently
struct StegOperationResult {
    bool success;
    std::string message;
    std::string output_path; // For decode, this will be the path to the secret file
};

StegOperationResult py_encode(const std::string &src_image_path,
                              const std::string &secret_file_path,
                              const std::string &stego_image_path,
                              const std::string &magic_string)
{
    EncodeInfo encInfo;
    memset(&encInfo, 0, sizeof(EncodeInfo)); // Initialize struct

    // Pybind11 strings are std::string, C functions need char*.
    // Use .c_str() for read-only, or copy if the C function might modify (not typical for paths).
    // We need to manage the lifetime of these char* if they are not just temporaries.
    // strdup is a good choice here, but remember to free.

    encInfo.src_image_fname = strdup(src_image_path.c_str());
    encInfo.secret_fname = strdup(secret_file_path.c_str());
    encInfo.stego_image_fname = strdup(stego_image_path.c_str());
    
    if (!encInfo.src_image_fname || !encInfo.secret_fname || !encInfo.stego_image_fname) {
        if(encInfo.src_image_fname) free(encInfo.src_image_fname);
        if(encInfo.secret_fname) free(encInfo.secret_fname);
        if(encInfo.stego_image_fname) free(encInfo.stego_image_fname);
        return {false, "Memory allocation failed for file paths.", ""};
    }

    Status status = open_files(&encInfo);
    if (status == e_failure) {
        free(encInfo.src_image_fname);
        free(encInfo.secret_fname);
        free(encInfo.stego_image_fname);
        return {false, "Failed to open input/output files for encoding.", ""};
    }

    // do_encoding now calls check_capacity internally after it has the magic string
    status = do_encoding(&encInfo, magic_string.c_str());

    // Clean up file pointers
    if (encInfo.fptr_src_image) fclose(encInfo.fptr_src_image);
    if (encInfo.fptr_secret) fclose(encInfo.fptr_secret);
    if (encInfo.fptr_stego_image) fclose(encInfo.fptr_stego_image);
    
    // Clean up strdup'd paths
    // encInfo.ext is freed inside do_encoding or check_capacity
    free(encInfo.src_image_fname);
    free(encInfo.secret_fname);
    free(encInfo.stego_image_fname);


    if (status == e_success) {
        return {true, "Encoding successful.", stego_image_path};
    } else {
        // More specific error messages could be propagated from C++ functions
        // For now, a generic failure message.
        // Remove output file if encoding failed to prevent partial/corrupt file
        remove(stego_image_path.c_str()); 
        return {false, "Encoding failed. Check image capacity and file integrity.", ""};
    }
}

StegOperationResult py_decode(const std::string &stego_image_path,
                              const std::string &output_secret_base_path, // e.g., "output/decoded_secret" (no ext)
                              const std::string &magic_string)
{
    EncodeInfo encInfo;
    memset(&encInfo, 0, sizeof(EncodeInfo));

    encInfo.stego_image_fname = strdup(stego_image_path.c_str());
    // dest_file path will be constructed after decoding extension
    // For now, store the base path. Python will provide a temporary name for the base.
    // char temp_dest_path_buffer[1024]; // Buffer for constructing full path
    // strncpy(temp_dest_path_buffer, output_secret_base_path.c_str(), sizeof(temp_dest_path_buffer) -1);
    // temp_dest_path_buffer[sizeof(temp_dest_path_buffer)-1] = '\0';
    // encInfo.dest_file = temp_dest_path_buffer; // This is risky if ext is too long.
                                             // Let's manage full path construction more carefully.
    
    // We will store the base path in encInfo.dest_file initially, and append the extension later.
    // The buffer for encInfo.dest_file needs to be large enough.
    // Or, better: encInfo.ext will be populated by decode_file_extension.
    // Then, we construct the final path here in C++ or return ext to Python.
    // Let's plan to construct the path here.

    char *final_output_path_c_str = nullptr;


    if (!encInfo.stego_image_fname) {
         if(encInfo.stego_image_fname) free(encInfo.stego_image_fname);
        return {false, "Memory allocation failed for stego image path.", ""};
    }
    
    Status status = open_decode_files(&encInfo); // Opens fptr_stego_image
    if (status == e_failure) {
        free(encInfo.stego_image_fname);
        return {false, "Failed to open stego image for decoding.", ""};
    }

    // The core do_decoding needs fptr_dest_file to be open.
    // Let's refine the flow:
    // 1. Open stego image.
    // 2. Seek past header.
    // 3. Extract magic (part of do_decoding).
    // 4. Decode extension (part of do_decoding, populates encInfo.ext).
    // 5. Construct full output path using output_secret_base_path and encInfo.ext.
    // 6. Open fptr_dest_file using this full path.
    // 7. Decode secret data.

    // To achieve this, we need to slightly restructure or make open_dest_file more flexible.
    // Let's try to call parts of do_decoding sequentially or ensure do_decoding handles this.
    // The current do_decoding expects fptr_dest_file to be NULL initially, 
    // and it calls open_dest_file internally after figuring out the extension.
    // However, decode_file_extension now just sets encInfo.ext.
    // So, we need to open the destination file *after* decode_file_extension and *before* decode_secret_data.

    // Modified plan:
    // Inside do_decoding, after decode_file_extension gets encInfo.ext:
    //   construct full_path = output_secret_base_path + encInfo.ext
    //   open_dest_file(&encInfo, full_path)
    
    // For this to work, pass output_secret_base_path to do_decoding, perhaps via EncodeInfo struct.
    // Let's add a temporary field to EncodeInfo or handle path construction here.
    // For now, let's assume `encInfo.dest_file` is the base path and `do_decoding`
    // will try to append `encInfo.ext` to it.
    // This means `encInfo.dest_file` must be a buffer.

    char dest_file_buffer[1024]; // Max path length
    strncpy(dest_file_buffer, output_secret_base_path.c_str(), sizeof(dest_file_buffer) - 1);
    dest_file_buffer[sizeof(dest_file_buffer) - 1] = '\0';
    encInfo.dest_file = dest_file_buffer; // Point to stack buffer, be careful

    // Temporarily, let's call parts of do_decoding to manage file opening properly.
    // This is a bit of a refactor of do_decoding's internal logic for the binding layer.

    std::string final_output_path_str;

    if (fseek(encInfo.fptr_stego_image, 54, SEEK_SET) != 0) {
        fclose(encInfo.fptr_stego_image);
        free(encInfo.stego_image_fname);
        return {false, "Failed to seek in stego image.", ""};
    }

    if (extract_magic(&encInfo, magic_string.c_str()) == e_failure) {
        fclose(encInfo.fptr_stego_image);
        free(encInfo.stego_image_fname);
        return {false, "Magic string validation failed.", ""};
    }

    if (decode_file_extension(&encInfo) == e_failure) { // This will set encInfo.ext
        fclose(encInfo.fptr_stego_image);
        free(encInfo.stego_image_fname);
        if(encInfo.ext) free(encInfo.ext);
        return {false, "Failed to decode file extension.", ""};
    }
    
    // Now encInfo.ext should be populated. Construct full output path.
    final_output_path_str = output_secret_base_path + (encInfo.ext ? encInfo.ext : "");
    final_output_path_c_str = strdup(final_output_path_str.c_str());

    if (!final_output_path_c_str) {
        fclose(encInfo.fptr_stego_image);
        free(encInfo.stego_image_fname);
        if(encInfo.ext) free(encInfo.ext);
        return {false, "Memory allocation for output path failed.", ""};
    }

    if (open_dest_file(&encInfo, final_output_path_c_str) == e_failure) { // Open the actual output file
        fclose(encInfo.fptr_stego_image);
        free(encInfo.stego_image_fname);
        if(encInfo.ext) free(encInfo.ext);
        free(final_output_path_c_str);
        return {false, "Failed to open destination file: " + final_output_path_str, ""};
    }

    status = decode_secret_data(&encInfo); // This writes to encInfo.fptr_dest_file

    // Clean up
    if (encInfo.fptr_stego_image) fclose(encInfo.fptr_stego_image);
    if (encInfo.fptr_dest_file) fclose(encInfo.fptr_dest_file);
    free(encInfo.stego_image_fname);
    if (encInfo.ext) free(encInfo.ext); // ext was strdup'd in decode_file_extension
    
    if (status == e_success) {
        std::string return_path = final_output_path_str;
        free(final_output_path_c_str);
        return {true, "Decoding successful.", return_path};
    } else {
        remove(final_output_path_c_str); // Remove potentially corrupt output file
        free(final_output_path_c_str);
        return {false, "Decoding failed.", ""};
    }
}


PYBIND11_MODULE(steganography_engine, m) {
    m.doc() = "Python bindings for C++ LSB Steganography";

    py::class_<StegOperationResult>(m, "StegOperationResult")
        .def_readonly("success", &StegOperationResult::success)
        .def_readonly("message", &StegOperationResult::message)
        .def_readonly("output_path", &StegOperationResult::output_path);

    m.def("encode", &py_encode, "Encodes a secret file into a source image",
          py::arg("src_image_path"),
          py::arg("secret_file_path"),
          py::arg("stego_image_path"),
          py::arg("magic_string"));

    m.def("decode", &py_decode, "Decodes a secret file from a stego image",
          py::arg("stego_image_path"),
          py::arg("output_secret_base_path"),
          py::arg("magic_string"));
}