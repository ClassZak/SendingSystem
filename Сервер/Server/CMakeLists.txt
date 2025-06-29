cmake_minimum_required(VERSION 3.28)

# Автоматическая настройка для Windows 7
if(WIN32 AND CMAKE_SYSTEM_VERSION VERSION_EQUAL 6.1)
	# Принудительная настройка для Windows 7
	set(CMAKE_GENERATOR "Ninja" CACHE INTERNAL "" FORCE)
	set(CMAKE_MAKE_PROGRAM "C:/gcc-10.4.0-no-debug/bin/ninja.exe" CACHE FILEPATH "Path to Ninja" FORCE)
	set(CMAKE_C_COMPILER "C:/gcc-10.4.0-no-debug/bin/gcc.exe" CACHE FILEPATH "C compiler" FORCE)
	set(CMAKE_C_COMPILER_WORKS 1 CACHE BOOL "" FORCE)
	set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY CACHE STRING "" FORCE)
	
	# Установка стандарта C11 для GCC
	set(CMAKE_C_STANDARD 11)
	set(CMAKE_C_STANDARD_REQUIRED ON)
endif()

project(SendingSystem VERSION 1.0
		DESCRIPTION "Application for data transfering"
		LANGUAGES C
)

# Добавление исходников
add_executable(
	SendingSystem 
	${CMAKE_SOURCE_DIR}/src/Server.c
	${CMAKE_SOURCE_DIR}/src/cJSON.c
	${CMAKE_SOURCE_DIR}/src/Server.h
	${CMAKE_SOURCE_DIR}/src/cJSON.h
)

# Платформенно-специфичные настройки
if(WIN32)
	target_link_libraries(SendingSystem PRIVATE ws2_32)
	
	# Флаги для Windows 7
	if(CMAKE_SYSTEM_VERSION VERSION_EQUAL 6.1)
		add_compile_definitions(_WIN32_WINNT=0x0601)
		target_compile_options(SendingSystem PRIVATE -O0)
	endif()
else()
	# Настройки для Linux/MacOS
	target_compile_options(SendingSystem PRIVATE -O0)
endif()

# Настройки для MSVC (Windows 10/11)
if(MSVC)
	# Активация политики для Edit and Continue
	cmake_policy(SET CMP0141 NEW)
	target_compile_options(SendingSystem PRIVATE "/std:c11")
	
	set_property(TARGET SendingSystem PROPERTY
		MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug>:EditAndContinue>"
	)
	set_target_properties(SendingSystem PROPERTIES
		LINK_FLAGS_DEBUG "/INCREMENTAL"
	)
	
	# Флаги оптимизации
	target_compile_options(SendingSystem PRIVATE
		"$<$<CONFIG:Debug>:/Od>"
	)
endif()
