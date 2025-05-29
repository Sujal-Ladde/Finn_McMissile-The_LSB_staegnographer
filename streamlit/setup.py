from setuptools import setup, Extension
import pybind11
import os
import platform # Import platform module

# Define compiler-specific arguments
if platform.system() == "Windows":
    cpp_args = ['/std:c++14', '/EHsc'] # MSVC: C++14 standard, EHsc for exception handling
    # MSVC default release build already includes /O2 (optimization)
    # Define _CRT_SECURE_NO_WARNINGS to suppress warnings about "unsafe" functions
    cpp_args.append('/D_CRT_SECURE_NO_WARNINGS')
else:
    cpp_args = ['-std=c++14', '-O3', '-Wall', '-fPIC'] # GCC/Clang

# All source files for the extension
sources = [
    'cpp_backend/bindings.cpp',
    'cpp_backend/src/common.cpp',
    'cpp_backend/src/encode.cpp',
    'cpp_backend/src/decode.cpp'
]

steganography_module = Extension(
    'steganography_engine', # Name of the module as imported in Python
    sources=sources,
    include_dirs=[
        pybind11.get_include(),
        'cpp_backend/include' # Path to your header files
    ],
    language='c++',
    extra_compile_args=cpp_args,
)

setup(
    name='steganography_engine',
    version='0.1',
    description='LSB Steganography engine with Python bindings',
    ext_modules=[steganography_module],
    author='Your Name', # Replace with your name
    author_email='your.email@example.com', # Replace with your email
    zip_safe=False,
)