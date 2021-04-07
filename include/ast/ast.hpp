#ifndef AST_HPP
#define AST_HPP

#include <memory>
#include <utility>
#include <ostream>

namespace ast {
class node
{
  public:
    explicit node(std::string name)
            : name_(std::move(name)), nodes_() {}

    void append_child(std::shared_ptr<node> node)
    {
        nodes_.push_back(std::move(node));
    }

    [[nodiscard]] const std::string &get_name() const noexcept { return name_; }

    [[nodiscard]] std::vector<std::shared_ptr<node>> nodes() const noexcept
    {
        if (!disabled)
        {
            return nodes_;
        }
        std::vector<std::shared_ptr<node>> nodes = {};
        for (auto &&n : nodes_)
        {
            if (!n)
            { continue; }
            for (auto &&g : n->nodes())
                nodes.push_back(g);
        }
        return nodes;
    }

    void disable() noexcept { disabled = true; }

  private:
    std::string name_{};
    std::vector<std::shared_ptr<node>> nodes_{};
    bool disabled = false;
};

[[nodiscard]] std::vector<std::shared_ptr<node>> nodes(const std::shared_ptr<node> &v)
{
    return v == nullptr ? std::vector<std::shared_ptr<node>>{} : v->nodes();
}

using node_ptr = std::shared_ptr<node>;

node_ptr make_node(const std::string &name)
{
    return std::make_shared<node>(name);
}

struct node_printer
{
    node_ptr v;
    std::size_t align;
};

namespace detail {
void print_n(std::ostream &os, std::size_t n, const std::string &s)
{
    for (std::size_t i = 0; i < n; ++i)
    {
        os << s;
    }
}
} // namespace detail

std::ostream &operator<<(std::ostream &os, const node_printer &printer)
{
    detail::print_n(os, printer.align, "\t");
    os << printer.v->get_name() << std::endl;
    for (const auto &ch : printer.v->nodes())
    {
        if (ch == nullptr)
        {
            continue;
        }
        os << node_printer{ch, printer.align + 1};
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const node_ptr &node)
{
    os << node_printer{node, 0};
    return os;
}
}

#endif // AST_HPP
