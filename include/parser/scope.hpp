#ifndef PARSER_SCOPE_HPP
#define PARSER_SCOPE_HPP

#include <cstdint>
#include <string>

#include "parser/input_reader.hpp"
#include "parser/position.hpp"

namespace parser {

struct scope
{
    std::shared_ptr<input_reader> reader;
    position pos;

    scope(std::shared_ptr<input_reader> reader, position pos)
            : reader(std::move(reader)), pos(pos)
    {
    }

    [[nodiscard]] exception::positional_error
    raise(const std::string &target, const std::string &explanation) const
    {
        return exception::positional_error(pos, "Can not " + target + ": " +
                                                explanation);
    }

    [[nodiscard]] exception::positional_error
    raise_expected(const std::string &expected) const
    {
        return exception::positional_error(pos, "'" + expected +
                                                "' is expected here");
    }

    [[nodiscard]] exception::positional_error raise_unexpected() const
    {
        return exception::positional_error(pos, "Unexpected token");
    }

    [[nodiscard]] exception::positional_error raise_eof() const
    {
        return exception::positional_error(pos, "Unexpected end of file");
    }

    char next_char() { return reader->read_and_move(pos); }

    bool has_next() { return reader->can_read(pos); }
};

} // namespace parser

#endif // PARSER_SCOPE_HPP
