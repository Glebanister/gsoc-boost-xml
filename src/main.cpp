#include "parser/combinators.hpp"

int main()
{
    using namespace parser;
    using namespace aliases;

    parser_ptr string_literal = m_seq(m_char('"'), m_any(m_not_char('"')), m_char('"'));
    parser_ptr non_string_literal = m_any(m_not_charset(',', '"'));

    parser_ptr cell = m_alt(string_literal, non_string_literal);
    parser_ptr row = m_separator(cell, m_char(','));

    std::shared_ptr<input_reader> reader = std::make_shared<full_file_reader>(
            "/home/gleb/Documents/projects/gsoc-boost-xml/input.txt");
    scope s(reader, position());

    assert_no_error(m_eof(m_separator(non_string_literal, m_char(',')))->parse(s));
}
