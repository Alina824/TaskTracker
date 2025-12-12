# Makefile для сборки и запуска TaskTrackerAndKalendar
# Требуется: GNU make, CMake, Qt6, MinGW

# Пути (настройте под вашу систему)
# Примеры: C:/msys64/ucrt64/bin или C:/Qt/6.10.1/mingw_64/bin
MINGW_PATH ?= C:/msys64/ucrt64/bin
QT_PATH ?= C:/Qt/6.10.1/mingw_64/bin

# Имя проекта
PROJECT_NAME = TaskTrackerAndKalendar

# Директория сборки
BUILD_DIR = build

# Исполняемый файл
EXECUTABLE = $(BUILD_DIR)/$(PROJECT_NAME).exe

# Генератор CMake (MinGW Makefiles для Windows)
CMAKE_GENERATOR ?= MinGW Makefiles

# Цвета для вывода (для совместимости убраны escape-последовательности)
# На Windows можно использовать echo без цветов или установить более продвинутый make

.PHONY: all build configure clean run rebuild help quick-build clean-objs run-with-qt

# Цель по умолчанию
all: build

# Конфигурация CMake
configure:
	@echo Configuring CMake...
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake -G "$(CMAKE_GENERATOR)" ..

# Сборка проекта
build: configure
	@echo Building project...
	@cd $(BUILD_DIR) && cmake --build . --config Release
	@echo Build complete!

# Быстрая сборка (без переконфигурации)
quick-build:
	@echo Building project...
	@cd $(BUILD_DIR) && cmake --build . --config Release
	@echo Build complete!

# Пересборка (очистка + сборка)
rebuild: clean build

# Запуск приложения
run: build
	@echo Running application...
	@cd $(BUILD_DIR) && ./$(PROJECT_NAME).exe

# Запуск с установкой PATH для Qt DLL
run-with-qt: build
	@echo Running application with Qt paths...
	@PATH="$(MINGW_PATH):$(QT_PATH):$$PATH" && cd $(BUILD_DIR) && ./$(PROJECT_NAME).exe

# Очистка директории сборки
clean:
	@echo Cleaning build directory...
	@rm -rf $(BUILD_DIR)
	@echo Clean complete!

# Частичная очистка (только объектные файлы, сохраняет CMake конфигурацию)
clean-objs:
	@echo Cleaning object files...
	@cd $(BUILD_DIR) && cmake --build . --target clean
	@echo Clean complete!

# Показать справку
help:
	@echo Available targets:
	@echo   all          - Configure and build project (default)
	@echo   build        - Configure and build project
	@echo   quick-build  - Build without reconfiguring (faster)
	@echo   configure    - Only configure CMake
	@echo   rebuild      - Clean and rebuild from scratch
	@echo   run          - Build and run application
	@echo   run-with-qt  - Build and run with Qt DLL paths set
	@echo   clean        - Remove build directory completely
	@echo   clean-objs   - Clean object files only (keeps CMake config)
	@echo   help         - Show this help message
	@echo.
	@echo Variables you can set:
	@echo   MINGW_PATH   - Path to MinGW bin directory (default: C:/msys64/ucrt64/bin)
	@echo   QT_PATH      - Path to Qt bin directory (default: C:/Qt/6.10.1/mingw_64/bin)
	@echo   CMAKE_GENERATOR - CMake generator (default: MinGW Makefiles)
	@echo.
	@echo Examples:
	@echo   make                    - Build project
	@echo   make run                - Build and run
	@echo   make clean              - Clean build directory
	@echo   make MINGW_PATH=C:/path/to/mingw/bin build - Build with custom MinGW path

