MINGW_PATH ?= C:/msys64/ucrt64/bin
QT_PATH ?= C:/Qt/6.10.1/mingw_64/bin

PROJECT_NAME = TaskTrackerAndKalendar

BUILD_DIR = build

EXECUTABLE = $(BUILD_DIR)/$(PROJECT_NAME).exe

CMAKE_GENERATOR ?= MinGW Makefiles

.PHONY: all build configure clean run rebuild quick-build clean-objs run-with-qt

all: build

configure:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake -G "$(CMAKE_GENERATOR)" ..

build: configure
	@cd $(BUILD_DIR) && cmake --build . --config Release

quick-build:
	@cd $(BUILD_DIR) && cmake --build . --config Release

rebuild: clean build

run: build
	@cd $(BUILD_DIR) && ./$(PROJECT_NAME).exe

run-with-qt: build
	@PATH="$(MINGW_PATH):$(QT_PATH):$$PATH" && cd $(BUILD_DIR) && ./$(PROJECT_NAME).exe

clean:
	@rm -rf $(BUILD_DIR)

clean-objs:
	@cd $(BUILD_DIR) && cmake --build . --target clean

