//
// Created by gleb on 07/04/2021.
//

#ifndef CSV_CSV_PARSER_HPP
#define CSV_CSV_PARSER_HPP

#include "parser/combinators.hpp"
#include "csv/csv_table.hpp"
#include "ast/ast.hpp"

namespace csv {

parser::parser_ptr csv_parser()
{
    using namespace parser;
    using namespace aliases;

    parser_ptr comma = m_ignore(m_char(','));
    parser_ptr quote = m_ignore(m_char('"'));

    parser_ptr string_literal = m_erase(m_concat(m_seq(quote,
                                                       m_concat(m_any(m_not_char('"'))),
                                                       quote)));
    parser_ptr non_string_literal = m_concat(m_any(m_not_charset(',', '"', '\n')));
    parser_ptr cell = m_alt(string_literal, non_string_literal);
    parser_ptr row = m_line(m_separator(cell, comma));
    parser_ptr rows = m_erase(m_any(row));
    parser_ptr csv = m_erase(m_eof(rows));

    return csv;
}

csv_table import_csv(const std::string &filename)
{
    using namespace parser;

    std::shared_ptr<input_reader> reader = std::make_shared<full_file_reader>(filename);
    scope s(reader, position());

    auto result = csv_parser()->parse(s);

    if (!::parser::no_error(result))
    {
        throw ::parser::get_error(result);
    }

    return csv_table(::parser::get_ast(result));
}
} // namespace csv

#endif //CSV_CSV_PARSER_HPP
