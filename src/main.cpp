#include <iostream>

#include "csv/csv_parser.hpp"

int main(int argc, const char **argv)
{
    if (argc < 2)
    {
        std::cout << "Specify path to csv file as first argument" << std::endl;
        return 0;
    }
    try
    {
        std::cout << csv::import_csv(argv[1]) << std::endl;
    } catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    } catch (...)
    {
        std::cerr << "Unknown error occurred" << std::endl;
    }
}
