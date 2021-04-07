#ifndef PARSER_PARSER_HPP
#define PARSER_PARSER_HPP

#include <memory>
#include <variant>
#include <unordered_set>
#include <utility>

#include "parser/scope.hpp"
#include "parser/collectors.hpp"

namespace parser {

template<typename ResT>
using maybe_error = std::variant<exception::parser_error, ResT>;

template<typename ResT>
inline bool no_error(const maybe_error<ResT> &ret) { return ret.index() == 1; }

inline void assert_no_error(const maybe_error &error)
{
    if (error.has_value())
    {
        throw error.value();
    }
}

class parser
{
  public:
    virtual maybe_error parse(scope &) = 0;
};

using parser_ptr = std::shared_ptr<parser>;

template<typename T, typename... Args>
parser_ptr make_parser(Args &&...args)
{
    return std::static_pointer_cast<parser>(
            std::make_shared<T>(std::forward<Args>(args)...));
}

class inner_parser_container_ : public parser
{
  public:
    explicit inner_parser_container_(parser_ptr inner)
            : inner_(std::move(inner))
    {
    }

  protected:
    parser_ptr inner_;
};

class position_rollback
{
  public:
    explicit position_rollback(scope &sc)
            : sc_(sc), ini_(sc.pos), canceled_(false)
    {
    }

    void cancel() noexcept { canceled_ = true; }

    ~position_rollback()
    {
        if (!canceled_)
            sc_.pos = ini_;
    }

  private:
    scope &sc_;
    position ini_;
    bool canceled_;
};

} // namespace parser
#endif // PARSER_PARSER_HPP
