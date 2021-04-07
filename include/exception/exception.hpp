#ifndef PARSER_EXCEPTION_HPP
#define PARSER_EXCEPTION_HPP

#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "parser/position.hpp"

namespace parser::exception {

namespace format {
static std::string caused_by(const std::string &cause) { return ": " + cause; }

static std::string because(const std::string &problem, const std::string &cause)
{
    return problem + caused_by(cause);
}
} // namespace format

class parser_error : public std::exception {
  public:
    explicit parser_error(const std::string &message)
        : message_(format::because("Parse error", message))
    {
    }

    [[nodiscard]] const char *what() const noexcept override
    {
        return message_.c_str();
    }

  protected:
    const std::string message_;
};

class positional_error : public parser_error {
  public:
    explicit positional_error(const position &pos, const std::string &message)
        : parser_error("at " + pos.format() + ": " + message), position_(pos)
    {
    }

    [[nodiscard]] const position &get_position() const noexcept
    {
        return position_;
    }

  protected:
    const position position_;
};

} // namespace parser::exception
#endif // PARSER_EXCEPTION_HPP
