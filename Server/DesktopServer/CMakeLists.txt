cmake_minimum_required(VERSION 3.28...4.4)






# Windows 7 settings
if(WIN32 AND CMAKE_SYSTEM_VERSION VERSION_EQUAL 6.1)
	set(CMAKE_GENERATOR "Ninja" CACHE INTERNAL "" FORCE)
	set(
		CMAKE_MAKE_PROGRAM
			"C:/gcc-10.4.0-no-debug/bin/ninja.exe"
			CACHE FILEPATH "Path to Ninja" FORCE
	)
	set(
		CMAKE_C_COMPILER
			"C:/gcc-10.4.0-no-debug/bin/gcc.exe"
			CACHE FILEPATH "C compiler" FORCE
	)
	set(CMAKE_C_COMPILER_WORKS 1 CACHE BOOL "" FORCE)
	set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY CACHE STRING "" FORCE)
	
	# C11 for GCC compiler
	set(CMAKE_C_STANDARD 11)
	set(CMAKE_C_STANDARD_REQUIRED ON)
endif()




project(SendingSystem VERSION 1.0
		DESCRIPTION "Application for data transfering"
		LANGUAGES C
)


# Fetch dependecies
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
include(FetchContent)
FetchContent_Declare(
	cJSON
	GIT_REPOSITORY	https://github.com/DaveGamble/cJSON.git
	GIT_TAG			v1.7.18 
)
FetchContent_MakeAvailable(cJSON)
# Find packeges
find_package(OpenSSL REQUIRED)
find_package(SQLite3 REQUIRED)






# Sources
add_executable(
	SendingSystem 
	${CMAKE_SOURCE_DIR}/src/Server.c
	${CMAKE_SOURCE_DIR}/src/Server.h
	${CMAKE_SOURCE_DIR}/src/PrintProcedures/PrintProcedures.h
	${CMAKE_SOURCE_DIR}/src/PrintProcedures/PrintProcedures.c
	${CMAKE_SOURCE_DIR}/src/FileService/FileService.h
	${CMAKE_SOURCE_DIR}/src/FileService/FileService.c
	${CMAKE_SOURCE_DIR}/src/Base64/Base64.h
	${CMAKE_SOURCE_DIR}/src/Base64/Base64.c
	${CMAKE_SOURCE_DIR}/src/OpenSSL/OpenSSL.h
	${CMAKE_SOURCE_DIR}/src/OpenSSL/OpenSSL.c
	${CMAKE_SOURCE_DIR}/src/Connection/connection.h
	${CMAKE_SOURCE_DIR}/src/Connection/connection_socket.h
	${CMAKE_SOURCE_DIR}/src/Connection/connection_socket.c
	${CMAKE_SOURCE_DIR}/src/Connection/connection_SSL.h
	${CMAKE_SOURCE_DIR}/src/Connection/connection_SSL.c
)

# Install dependecies
target_link_libraries(SendingSystem 
	PRIVATE
	cjson
	OpenSSL::SSL
	OpenSSL::Crypto
	SQLite::SQLite3
)
target_include_directories(SendingSystem PRIVATE ${cJSON_SOURCE_DIR})
get_target_property(SendingSystem_INCLUDES SendingSystem INCLUDE_DIRECTORIES)
message(STATUS "SendingSystem includes: ${SendingSystem_INCLUDES}")
if(TARGET cjson)
    message(STATUS "Target cjson EXISTS")
else()
    message(FATAL_ERROR "Target cjson NOT FOUND")
endif()





# Platform orient settings
if(WIN32)
	target_link_libraries(SendingSystem PRIVATE ws2_32)
	
	# Windows 7 flags
	if(CMAKE_SYSTEM_VERSION VERSION_EQUAL 6.1)
		add_compile_definitions(_WIN32_WINNT=0x0601)
		target_compile_options(SendingSystem PRIVATE -O0)
	endif()
else()
	# Linux and MacOS settings
	target_compile_options(SendingSystem PRIVATE -O0)
endif()

# MSVC settings (Windows 10/11)
if(MSVC)
	# Activate Edit and Continue policy
	cmake_policy(SET CMP0141 NEW)
	target_compile_options(SendingSystem PRIVATE "/std:c11")
	
	set_property(TARGET SendingSystem PROPERTY
		MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug>:EditAndContinue>"
	)
	set_target_properties(SendingSystem PROPERTIES
		LINK_FLAGS_DEBUG "/INCREMENTAL"
	)
	
	# No optimization flags
	target_compile_options(SendingSystem PRIVATE
		"$<$<CONFIG:Debug>:/Od>"
	)

endif()
