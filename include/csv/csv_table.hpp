//
// Created by gleb on 07/04/2021.
//

#ifndef CSV_CSV_TABLE_HPP
#define CSV_CSV_TABLE_HPP

#include "ast/ast.hpp"

namespace csv {

class csv_table
{
  public:
    csv_table() = default;

    explicit csv_table(ast::node_ptr n)
            : table()
    {
        for (const auto &row_node : ast::nodes(n))
        {
            if (!row_node)
            {
                continue;
            }
            std::vector<std::string> row;
            for (const auto &cell_node : ast::nodes(row_node))
            {
                row.push_back(cell_node->get_name());
            }
            add_row(row);
        }
    }

    [[nodiscard]] std::size_t height() const noexcept
    {
        return table.size();
    }

    [[nodiscard]] std::size_t width() const noexcept
    {
        return table.empty() ? 0 : table[0].size();
    }

    [[nodiscard]] bool can_add_row(const std::vector<std::string> &row) const noexcept
    {
        return table.empty() || table[0].size() == row.size();
    }

    void add_row(const std::vector<std::string> &row)
    {
        if (!can_add_row(row))
        {
            throw std::logic_error("Row length does not correspond to table width");
        }
        table.push_back(row);
    }

    [[nodiscard]] const std::vector<std::vector<std::string>> &rows() const noexcept
    {
        return table;
    }

  private:
    std::vector<std::vector<std::string>> table;
};

std::ostream &operator<<(std::ostream &os, const csv_table &table)
{
    for (const auto &row : table.rows())
    {
        for (const std::string &s : row)
        {
            os << '\'' << s << '\'' << ' ';
        }
        os << '\n';
    }
    return os;
}

} // namespace csv

#endif //CSV_CSV_TABLE_HPP
