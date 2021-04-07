cmake_minimum_required(VERSION 3.17)
project(parse-csv)
set(CMAKE_CXX_STANDARD 20)

set(EXE parse-csv)
add_executable(${EXE} src/main.cpp include/parser/collectors.hpp include/csv/csv_table.hpp include/csv/csv_parser.hpp include/ast/ast.hpp)
target_include_directories(${EXE} PUBLIC include)
target_link_libraries(${EXE} stdc++)
target_link_libraries(${EXE} m)
