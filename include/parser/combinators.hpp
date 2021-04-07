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
        if (auto err = inner_->parse(sc); !no_error(err))
        {
            return err;
        }
        rollback.cancel();
        return std::nullopt;
    }
};

template<std::predicate<char> PredicateT>
class predicate_parser : public parser
{
  public:
    explicit predicate_parser(PredicateT predicate, const std::string &name)
            : predicate_(std::move(predicate)), name_(name) {}

    maybe_error parse(scope &sc) override
    {
        if (!sc.has_next())
            return sc.raise_eof();

        if (!predicate_(sc.next_char()))
            return sc.raise_expected(name_);

        return std::nullopt;
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

        return std::nullopt;
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
        for (std::size_t i = 0; i < at_least_; ++i)
        {
            if (auto err = inner_->parse(sc); !no_error(err))
            {
                return err;
            }
        }
        while (no_error(try_inner_->parse(sc)))
        {
        }
        return std::nullopt;
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
        for (auto &&item : sequence_)
        {
            if (auto err = item->parse(sc); !no_error(err))
            {
                return err;
            }
        }
        return std::nullopt;
    }

  private:
    std::vector<parser_ptr> sequence_;
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
        if (auto err = left_->parse(sc); !no_error(err))
        {
            if (auto r_err = right_->parse(sc); !no_error(err))
            {
                return r_err;
            }
        }
        return std::nullopt;
    }

  private:
    parser_ptr left_;
    parser_ptr right_;
};

namespace aliases {

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
    return m_seq(value, m_any(m_seq(sep, value)));
}

inline parser_ptr m_alt(const parser_ptr &left, const parser_ptr &right)
{
    return make_parser<alt_parser>(left, right);
}

inline parser_ptr m_eof(const parser_ptr &p) { return m_seq(p, make_parser<eof_parser>()); }

} // namespace aliases
} // namespace parser

#endif // PARSER_COMBINATORS_HPP
