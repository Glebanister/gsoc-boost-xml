#ifndef PARSER_COLLECTOR_HPP
#define PARSER_COLLECTOR_HPP

#include <string>

namespace parser::collector {

template<typename CollectorT, typename StreamT, typename OutT>
concept collector =
requires(CollectorT collector) {
    { collector.collect() } -> OutT;
    { collector.append(StreamT c) } -> void;
};

template<typename T>
struct value_holder
{
    T value;
};


struct char_collector : value_holder<char>
{
    char collect() { return c; }

    void append(char c) { value = c; }
};

struct string_collector : value_holder<std::string>
{
    std::string collect() { return value; }

    void append(char c) { value += c; }
};

template<typename T>
struct vector_collector : value_holder<std::vector<T>>
{
    std::vector<T> collect() { return value; }

    void append(T t) { value.push_back(std::move(t)); }
};

} // namespace parser::collector

#endif //PARSER_COLLECTOR_HPP
