# Makefile (ensure commands under targets are indented with a TAB)

PYTHON = python

.PHONY: all clean build_py run_streamlit

all: build_py

build_py:
	@echo "Building Python extension using setup.py..."
	$(PYTHON) setup.py build_ext --inplace
	@echo "Build complete."

clean:
	@echo "Cleaning up..."
	# Remove build artifacts created by setup.py
	@if exist build (rmdir /s /q build)
	@if exist dist (rmdir /s /q dist)
	@if exist steganography_engine.egg-info (rmdir /s /q steganography_engine.egg-info)
	@echo "Removing __pycache__ directories and .pyc files..."
	@for /r %%d in (__pycache__) do @if exist "%%d" (rmdir /s /q "%%d")
	@for /r %%f in (*.pyc) do @if exist "%%f" (del /q "%%f")
	@echo "Removing compiled extension..."
	@for /f "tokens=*" %%g in ('dir /b /s steganography_engine*.pyd 2^>nul') do @if exist "%%g" (del "%%g")
	@echo "Cleaning Streamlit temporary files (optional)..."
	@if exist streamlit_app\uploads (for /f "delims=" %%i in ('dir /b streamlit_app\uploads\*.* 2^>nul') do @if exist "streamlit_app\uploads\%%i" (del /q "streamlit_app\uploads\%%i"))
	@if exist streamlit_app\outputs (for /f "delims=" %%i in ('dir /b streamlit_app\outputs\*.* 2^>nul') do @if exist "streamlit_app\outputs\%%i" (del /q "streamlit_app\outputs\%%i"))
	@echo "Clean complete."

run_streamlit: build_py
	@echo "Running Streamlit app..."
	@cd streamlit_app && $(PYTHON) -m streamlit run app.py