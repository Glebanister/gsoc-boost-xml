#include <iostream>

#include "csv/csv_parser.hpp"

int main(int argc, const char **argv)
{
    if (argc < 2)
    {
        std::cout << "Specify path to csv file as first argument" << std::endl;
        return 0;
    }
    std::cout << csv::import_csv(argv[1]) << std::endl;
}
