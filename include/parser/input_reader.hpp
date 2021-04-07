#ifndef PARSER_INPUT_READER
#define PARSER_INPUT_READER

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

#include "exception/exception.hpp"
#include "parser/scope.hpp"

namespace parser {

class input_reading_error : public exception::parser_error
{
  public:
    explicit input_reading_error(const std::string &input_info)
            : parser_error("Can not read from " + input_info)
    {
    }

    explicit input_reading_error(const std::string &input_info,
                                 const std::string &message)
            : parser_error("Can not read from " + input_info + ": ")
    {
    }
};

class input_out_of_range_error : public input_reading_error
{
  public:
    explicit input_out_of_range_error(const std::string &input_info,
                                      const position &pos)
            : input_reading_error("Can not access scope " + pos.format())
    {
    }
};

class input_reader
{
  public:
    int read_at(const position &pos)
    {
        if (!can_read(pos))
        {
            throw input_out_of_range_error(input_info_, pos);
        }
        return read_char_if_can(pos);
    }

    char read_and_move(position &pos)
    {
        char symb = read_at(pos);
        pos.next_char();
        if (symb == '\n')
        {
            pos.next_line();
        }
        return symb;
    }

    virtual bool can_read(const position &) = 0;

    [[nodiscard]] const std::string &get_info() const noexcept
    {
        return input_info_;
    }

  protected:
    virtual char read_char_if_can(const position &) = 0;

    const std::string input_info_;
};

class full_file_reader : public input_reader
{
  public:
    explicit full_file_reader(const std::string &file_path)
    {
        std::ifstream input(file_path);
        if (!input.is_open())
        {
            throw input_reading_error(file_path);
        }
        input >> content_;
        std::cout << content_ << std::endl;
    }

    bool can_read(const position &pos) override
    {
        return pos.get_abs_pos() < content_.length();
    }

  private:
    char read_char_if_can(const position &pos) override
    {
        return content_[pos.get_abs_pos()];
    }

    std::string content_;
};

} // namespace parser

#endif // PARSER_INPUT_READER
