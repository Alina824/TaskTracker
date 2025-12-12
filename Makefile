MINGW_PATH ?= C:/msys64/ucrt64/bin
QT_PATH ?= C:/Qt/6.10.1/mingw_64/bin

PROJECT_NAME = TaskTrackerAndKalendar

BUILD_DIR = build

EXECUTABLE = $(BUILD_DIR)/$(PROJECT_NAME).exe

CMAKE_GENERATOR ?= MinGW Makefiles

.PHONY: all build configure clean run rebuild quick-build clean-objs run-with-qt

all: build

configure:
	@echo Configuring CMake...
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake -G "$(CMAKE_GENERATOR)" ..

build: configure
	@echo Building project...
	@cd $(BUILD_DIR) && cmake --build . --config Release
	@echo Build complete!

quick-build:
	@echo Building project...
	@cd $(BUILD_DIR) && cmake --build . --config Release
	@echo Build complete!

rebuild: clean build

run: build
	@echo Running application...
	@cd $(BUILD_DIR) && ./$(PROJECT_NAME).exe

run-with-qt: build
	@echo Running application with Qt paths...
	@PATH="$(MINGW_PATH):$(QT_PATH):$$PATH" && cd $(BUILD_DIR) && ./$(PROJECT_NAME).exe

# Очистка директории сборки
clean:
	@echo Cleaning build directory...
	@rm -rf $(BUILD_DIR)
	@echo Clean complete!

clean-objs:
	@echo Cleaning object files...
	@cd $(BUILD_DIR) && cmake --build . --target clean
	@echo Clean complete!

	@echo   make run                - Build and run
	@echo   make clean              - Clean build directory
	@echo   make MINGW_PATH=C:/path/to/mingw/bin build - Build with custom MinGW path

