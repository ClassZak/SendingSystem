cmake_minimum_required(VERSION 3.28)

# Горячая перезагрузка для MSVC
if(POLICY CMP0141)
	cmake_policy(SET CMP0141 NEW)
	if(MSVC)
		set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
			"$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,
            $<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,
            $<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>"
		)
	endif()
endif()

project(SendingSystem VERSION 1.0
		DESCRIPTION "Application for data transfering"
		LANGUAGES C
)

set(CMAKE_C_STANDART 11)

add_executable(
		SendingSystem
		${CMAKE_SOURSE_DIR}/src/SendingSystem.c
)

target_include_directories(
		SendingSystem,
		PUBLIC
		${CMAKE_SOURSE_DIR}/include
)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Выберите тип сборки (Debug или Release)" FORCE)
endif()

message(STATUS "Тип сборки: ${CMAKE_BUILD_TYPE}")


# Отключение оптимизаций
if(MSVC)
	target_compile_options(SendingSystem PRIVATE /utf8
		$<$<CONFIG:Debug>:/Od>
		$<$<CONFIG:Release>:/Od>
		$<$<CONFIG:RelWithDebInfo>:/Od>
		$<$<CONFIG:MinSizeRel>:/Od>
	)
else()
	# Другие компиляторы
endif()