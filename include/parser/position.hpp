#ifndef PARSER_POSITION_HPP
#define PARSER_POSITION_HPP

namespace parser {

class position
{
  public:
    explicit position() : line(0), pos(0), abs_pos(0) {}

    position(const position &) = default;

    position(position &&) = default;

    position &operator=(const position &) = default;

    position &operator=(position &&) = default;

    void next_line() noexcept
    {
        ++line;
        pos = 0;
    }

    void next_char() noexcept
    {
        ++pos;
        ++abs_pos;
    }

    [[nodiscard]] std::string format() const noexcept
    {
        return std::to_string(line) + ":" + std::to_string(pos);
    }

    [[nodiscard]] std::size_t get_abs_pos() const noexcept { return abs_pos; }

  private:
    std::size_t line, pos, abs_pos;
};

} // namespace parser

#endif // PARSER_POSITION_HPP
