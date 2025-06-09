cmake_minimum_required(VERSION 3.28)

project(SendingSystem VERSION 1.0
		DESCRIPTION "Application for data transfering"
		LANGUAGES C
)


add_executable(
		SendingSystem 
		${CMAKE_SOURCE_DIR}/src/Server.c
		${CMAKE_SOURCE_DIR}/src/cJSON.c
		${CMAKE_SOURCE_DIR}/src/Server.h
		${CMAKE_SOURCE_DIR}/src/cJSON.h
)



# Горячая перезагрузка для MSVC
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
endif()

# Установка флагов отключения оптимизации для всех конфигураций
if(MSVC)
	# Для MSVC используем /Od и отключаем другие оптимизации
	target_compile_options(SendingSystem PRIVATE
		"$<$<CONFIG:Debug>:/Od>"
	)
else()
	# Для GCC/Clang используем -O0
	target_compile_options(SendingSystem PRIVATE -O0)
endif()