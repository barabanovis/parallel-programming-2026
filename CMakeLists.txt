cmake_minimum_required(VERSION 3.12)

project("lab3_parallel" CXX)


find_package(MPI REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable("lab3_parallel" "main.cpp" "Matrix.h" )

# Подключение заголовочных файлов MPI
target_include_directories("lab3_parallel" PRIVATE ${MPI_INCLUDE_PATH})

# Линковка с библиотеками MPI
target_link_libraries("lab3_parallel" PRIVATE ${MPI_C_LIBRARIES})

# Установка компиляторов MPI (опционально)
if(MPI_C_COMPILER)
    set_target_properties("lab3_parallel" PROPERTIES
        CXX_COMPILER ${MPI_CXX_COMPILER}
        C_COMPILER ${MPI_C_COMPILER}
    )
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" OR
   CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR
   CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(${PROJECT_NAME} PRIVATE "-Wall" "-Weffc++" "-Wextra" "-Wpedantic")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  endif()
  target_compile_options(${PROJECT_NAME} PRIVATE "/W4" "/utf-8")
endif()
