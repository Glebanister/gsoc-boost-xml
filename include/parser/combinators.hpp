#ifndef PARSER_COMBINATORS_HPP
#define PARSER_COMBINATORS_HPP

#include <concepts>
#include <functional>
#include <utility>

#include "parser/parser.hpp"

namespace parser {
namespace detail {
std::string concat_charset(const std::unordered_set<char> &charset)
{
    std::stringstream chars;
    for (char c : charset) chars.put(c);
    return chars.str();
}
} // namespace detail

class try_parser : public inner_parser_container_
{
  public:
    using inner_parser_container_::inner_parser_container_;

    maybe_error parse(scope &sc) override
    {
        position_rollback rollback(sc);
        auto result = inner_->parse(sc);
        if (!no_error(result))
            return get_error(result);

        rollback.cancel();
        return get_ast(result);
    }
};

template<std::predicate<char> PredicateT>
class predicate_parser : public parser
{
  public:
    explicit predicate_parser(PredicateT predicate, std::string name)
            : predicate_(std::move(predicate)), name_(std::move(name)) {}

    maybe_error parse(scope &sc) override
    {
        if (!sc.has_next())
            return sc.raise_eof();

        char c = sc.next_char();

        if (!predicate_(c))
            return sc.raise_expected(name_);

        return ast::make_node(std::string{c});
    }

  private:
    PredicateT predicate_;
    const std::string name_;
};

class eof_parser : public parser
{
  public:
    maybe_error parse(scope &sc) override
    {
        if (sc.has_next())
            return sc.raise_expected("EOF");

        return ast::make_node("EOF");
    }
};

class char_parser : public predicate_parser<std::function<bool(char)>>
{
  public:
    explicit char_parser(char c)
            : predicate_parser([c](char got) { return got == c; },
                               std::string{c}) {}
};

class not_char_parser : public predicate_parser<std::function<bool(char)>>
{
  public:
    explicit not_char_parser(char c)
            : predicate_parser([c](char got) { return got != c; },
                               std::string{"not '"} + c + "'") {}
};

class charset_parser : public predicate_parser<std::function<bool(char)>>
{
  public:
    explicit charset_parser(std::unordered_set<char> charset)
            : predicate_parser([charset{std::move(charset)}](char got) {
                                   return charset.find(got) != charset.end();
                               },
                               "charset " + detail::concat_charset(charset)) {}
};

class not_charset_parser : public predicate_parser<std::function<bool(char)>>
{
  public:
    explicit not_charset_parser(std::unordered_set<char> charset)
            : predicate_parser([charset{std::move(charset)}](char got) {
        return charset.find(got) == charset.end();
    }, "not any char from " + detail::concat_charset(charset)) {}
};

class at_least_parser : public inner_parser_container_
{
  public:
    explicit at_least_parser(std::size_t at_least, const parser_ptr &pattern)
            : inner_parser_container_(pattern),
              at_least_(at_least),
              try_inner_(make_parser<try_parser>(pattern))
    {
    }

    maybe_error parse(scope &sc) override
    {
        ast::node_ptr node = ast::make_node("At least " + std::to_string(at_least_));
        for (std::size_t i = 0; i < at_least_; ++i)
        {
            auto result = inner_->parse(sc);
            if (!no_error(result))
                return get_error(result);

            node->append_child(get_ast(result));
        }
        bool success = true;
        while (success)
        {
            auto result = try_inner_->parse(sc);
            if (!no_error(result))
                success = false;
            else
                node->append_child(get_ast(result));
        }
        return node;
    }

  private:
    std::size_t at_least_;
    parser_ptr try_inner_;
};

class any_parser : public at_least_parser
{
  public:
    explicit any_parser(const parser_ptr &inner)
            : at_least_parser(0, inner) {}
};

class seq_parser : public parser
{
  public:
    explicit seq_parser(std::vector<parser_ptr> sequence)
            : sequence_(std::move(sequence))
    {
    }

    maybe_error parse(scope &sc) override
    {
        ast::node_ptr node = ast::make_node("Sequence");
        for (auto &&item : sequence_)
        {
            auto result = item->parse(sc);
            if (!no_error(result))
            {
                return get_error(result);
            }
            node->append_child(get_ast(result));
        }
        return node;
    }

  private:
    std::vector<parser_ptr> sequence_;
};

class separator_parser : public parser
{
  public:
    explicit separator_parser(parser_ptr value, parser_ptr sep)
            : value_(std::move(value)), sep_(std::move(sep))
    {
    }

    maybe_error parse(scope &sc) override
    {
        ast::node_ptr node = ast::make_node("Separator");
        auto result = value_->parse(sc);
        if (!no_error(result))
            return get_error(result);
        node->append_child(get_ast(result));
        while (true)
        {
            position_rollback rollback(sc);
            auto sep_result = sep_->parse(sc);
            if (!no_error(sep_result))
                return node;
            auto value_result = value_->parse(sc);
            if (!no_error(value_result))
                return node;
            node->append_child(get_ast(value_result));
            rollback.cancel();
        }
    }

  private:
    parser_ptr value_, sep_;
};

class alt_parser : public parser
{
  public:
    explicit alt_parser(parser_ptr left, parser_ptr right)
            : left_(make_parser<try_parser>(std::move(left))),
              right_(std::move(right))
    {
    }

    maybe_error parse(scope &sc) override
    {
        auto left_res = left_->parse(sc);
        if (no_error(left_res))
            return get_ast(left_res);

        auto right_res = right_->parse(sc);
        if (no_error(right_res))
            return get_ast(right_res);

        return get_error(left_res);
    }

  private:
    parser_ptr left_;
    parser_ptr right_;
};

class ignore_parser : public inner_parser_container_
{
  public:
    using inner_parser_container_::inner_parser_container_;

    maybe_error parse(scope &sc) override
    {
        auto result = inner_->parse(sc);
        if (!no_error(result))
            return get_error(result);
        return nullptr;
    }
};

class concat_parser : public inner_parser_container_
{
  public:
    using inner_parser_container_::inner_parser_container_;

    maybe_error parse(scope &sc) override
    {
        auto result = inner_->parse(sc);
        if (!no_error(result))
            return get_error(result);
        std::stringstream ss;
        for (auto &&sn : ast::nodes(get_ast(result)))
        {
            if (!sn) continue;
            ss << sn->get_name();
        }
        return ast::make_node(ss.str());
    }
};

class erase_parser : public inner_parser_container_
{
  public:
    using inner_parser_container_::inner_parser_container_;

    maybe_error parse(scope &sc) override
    {
        auto result = inner_->parse(sc);
        if (!no_error(result))
            return get_error(result);
        auto node = get_ast(result);
        node->disable();
        return node;
    }
};

namespace aliases {

inline parser_ptr m_concat(const parser_ptr &p) { return make_parser<concat_parser>(p); }

inline parser_ptr m_ignore(const parser_ptr &p) { return make_parser<ignore_parser>(p); }

inline parser_ptr m_erase(const parser_ptr &p) { return make_parser<erase_parser>(p); }

inline parser_ptr m_try(const parser_ptr &p) { return make_parser<try_parser>(p); }

inline parser_ptr m_char(char c) { return make_parser<char_parser>(c); }

inline parser_ptr m_not_char(char c) { return make_parser<not_char_parser>(c); }

inline parser_ptr m_charset(std::unordered_set<char> s) { return make_parser<charset_parser>(std::move(s)); }

inline parser_ptr m_not_charset(std::unordered_set<char> s) { return make_parser<not_charset_parser>(std::move(s)); }

template<typename... Args>
inline auto m_charset(Args &&... args) -> std::enable_if_t<(std::is_convertible_v<Args, char> && ...), parser_ptr>
{
    return m_charset(std::unordered_set<char>{{std::forward<Args>(args)...}});
}

template<typename... Args>
inline auto m_not_charset(Args &&... args) -> std::enable_if_t<(std::is_convertible_v<Args, char> && ...), parser_ptr>
{
    return m_not_charset(std::unordered_set<char>{{std::forward<Args>(args)...}});
}

inline parser_ptr m_at_least(std::size_t cnt, const parser_ptr &p) { return make_parser<at_least_parser>(cnt, p); }

inline parser_ptr m_many1(const parser_ptr &p) { return m_at_least(1, p); }

inline parser_ptr m_any(const parser_ptr &p) { return make_parser<any_parser>(p); }

inline parser_ptr m_seq(std::vector<parser_ptr> sequence) { return make_parser<seq_parser>(std::move(sequence)); }

template<typename... Args>
inline auto m_seq(Args &&... args) -> std::enable_if_t<(std::is_convertible_v<Args, parser_ptr> && ...), parser_ptr>
{
    return make_parser<seq_parser>(std::vector<parser_ptr>{{std::forward<Args>(args)...}});
}

inline parser_ptr m_separator(const parser_ptr &value, const parser_ptr &sep)
{
    return make_parser<separator_parser>(value, sep);
}

inline parser_ptr m_alt(const parser_ptr &left, const parser_ptr &right)
{
    return make_parser<alt_parser>(left, right);
}

inline parser_ptr m_eof(const parser_ptr &p) { return m_seq(p, make_parser<eof_parser>()); }

inline parser_ptr m_line(const parser_ptr &p) { return m_seq(p, m_ignore(m_char('\n'))); }


} // namespace aliases
} // namespace parser

#endif // PARSER_COMBINATORS_HPP
