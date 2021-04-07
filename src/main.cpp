#include <iostream>

#include "csv/csv_parser.hpp"

int main()
{
    csv::safe_parse_csv("/home/gleb/Documents/projects/gsoc-boost-xml/input.txt", std::cout);
}
