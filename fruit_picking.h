#ifndef FRUIT_PICKING_H
#define FRUIT_PICKING_H

#include <algorithm>
#include <compare>
#include <deque>
#include <iostream>
#include <functional>
#include <queue>
#include <ranges>
#include <string>
#include <vector>

constexpr const char* DEFAULT_PICKER_NAME = "Anonim";

enum class Taste { SWEET, SOUR };

enum class Size { LARGE, MEDIUM, SMALL };

enum class Quality { HEALTHY, ROTTEN, WORMY };

class Fruit {
   public:
    explicit constexpr Fruit(Taste taste, Size size, Quality quality);
    constexpr Fruit(const Fruit& other);
    explicit constexpr Fruit(std::tuple<Taste, Size, Quality>);
    constexpr Fruit& operator=(const Fruit&) = default;
    constexpr Fruit& operator=(Fruit&&) = default;
    void go_rotten();
    void become_worm_infested();
    constexpr Taste taste() const { return fruit_taste; }
    constexpr Size size() const { return fruit_size; }
    constexpr Quality quality() const { return fruit_quality; }
    constexpr bool operator==(const Fruit& other) const = default;
    inline explicit operator std::tuple<Taste, Size, Quality>() const {
        return {taste(), size(), quality()};
    }
    friend std::ostream& operator<<(std::ostream& os, const Fruit& fruit);

   private:
    Taste fruit_taste;
    Size fruit_size;
    Quality fruit_quality;
};

class Picker {
   public:
    Picker(std::string_view = DEFAULT_PICKER_NAME);
    const std::string& get_name() const { return picker_name; }
    std::size_t count_fruits() const { return collected_fruits.size(); }
    std::size_t count_taste(Taste taste) const;
    std::size_t count_size(Size size) const;
    std::size_t count_quality(Quality quality) const;
    Picker& operator+=(const Fruit& fruit);
    Picker& operator+=(Picker& other);
    Picker& operator-=(Picker& other);
    bool operator==(const Picker& other) const;
    auto operator<=>(const Picker& other) const;
    friend std::ostream& operator<<(std::ostream& os, const Picker& picker);

   private:
    std::string picker_name;
    std::deque<Fruit> collected_fruits;
    std::deque<Fruit>::size_type last_wormy_index =
        std::deque<Fruit>::size_type(-1);
    void adjust_index_after_pop_front();
    void handle_rot_between_last_two();
    void handle_worm_infection();
};

class Ranking {
   public:
    Ranking() = default;
    Ranking(const Ranking& other);
    Ranking& operator=(const Ranking& other) = default;
    Ranking(const std::initializer_list<Picker>& pickers_list);
    std::size_t count_pickers() const { return pickers.size(); };
    friend std::ostream& operator<<(std::ostream& os, const Ranking& ranking);
    Ranking& operator+=(const Picker& picker);
    Ranking& operator-=(const Picker& picker);
    Ranking& operator+=(const Ranking& other);
    Ranking operator+(const Ranking& other) const;
    const Picker& operator[](std::size_t index) const;

   private:
    std::vector<Picker> pickers;
};

constexpr Fruit::Fruit(Taste taste, Size size, Quality quality)
    : fruit_taste(taste), fruit_size(size), fruit_quality(quality) {}

constexpr Fruit::Fruit(const Fruit& other)
    : fruit_taste(other.fruit_taste),
      fruit_size(other.fruit_size),
      fruit_quality(other.fruit_quality) {}

constexpr Fruit::Fruit(std::tuple<Taste, Size, Quality> tpl)
    : fruit_taste(std::get<0>(tpl)),
      fruit_size(std::get<1>(tpl)),
      fruit_quality(std::get<2>(tpl)) {}

inline void Fruit::become_worm_infested() {
    if (fruit_quality == Quality::HEALTHY) {
        fruit_quality = Quality::WORMY;
    }
}

inline void Fruit::go_rotten() {
    if (fruit_quality == Quality::HEALTHY) {
        fruit_quality = Quality::ROTTEN;
    }
}

inline std::ostream& operator<<(std::ostream& os, const Fruit& fruit) {
    switch (fruit.taste()) {
        case Taste::SWEET:
            os << "[słodki";
            break;
        case Taste::SOUR:
            os << "[kwaśny";
            break;
    }
    switch (fruit.size()) {
        case Size::LARGE:
            os << " duży";
            break;
        case Size::MEDIUM:
            os << " średni";
            break;
        case Size::SMALL:
            os << " mały";
            break;
    }
    switch (fruit.quality()) {
        case Quality::HEALTHY:
            os << " zdrowy]";
            break;
        case Quality::ROTTEN:
            os << " nadgniły]";
            break;
        case Quality::WORMY:
            os << " robaczywy]";
            break;
    }
    return os;
}

inline Picker::Picker(std::string_view name)
    : picker_name(name.empty() ? DEFAULT_PICKER_NAME : std::string(name)) {}

inline std::size_t Picker::count_taste(Taste taste) const {
    return std::count_if(collected_fruits.begin(), collected_fruits.end(),
                         [taste](const auto& fruit) { return fruit.taste() == taste; });
}

inline std::size_t Picker::count_size(Size size) const {
    return std::count_if(collected_fruits.begin(), collected_fruits.end(),
                         [size](const auto& fruit) { return fruit.size() == size; });
}

inline std::size_t Picker::count_quality(Quality quality) const {
    return std::count_if(collected_fruits.begin(), collected_fruits.end(),
                         [quality](const auto& fruit) { return fruit.quality() == quality; });
}

inline std::ostream& operator<<(std::ostream& os, const Picker& picker) {
    os << picker.picker_name << ":\n";
    for (const auto& fruit : picker.collected_fruits) {
        os << "\t" << fruit << "\n";
    }
    return os;
}

inline bool Picker::operator==(const Picker& other) const {
    return picker_name == other.picker_name &&
           std::equal(collected_fruits.begin(), collected_fruits.end(),
                      other.collected_fruits.begin(), other.collected_fruits.end());
}

inline Picker& Picker::operator+=(const Fruit& fruit) {
    collected_fruits.push_back(fruit);
    handle_rot_between_last_two();
    handle_worm_infection();
    return *this;
}

inline void Picker::handle_rot_between_last_two() {
    if (collected_fruits.size() < 2) return;

    Fruit& last = collected_fruits.back();
    Fruit& second_last = collected_fruits[collected_fruits.size() - 2];

    if (last.quality() == Quality::ROTTEN && second_last.quality() == Quality::HEALTHY) {
        second_last.go_rotten();
    } else if (last.quality() == Quality::HEALTHY && second_last.quality() == Quality::ROTTEN) {
        last.go_rotten();
    }
}

inline void Picker::handle_worm_infection() {
    if (collected_fruits.empty()) return;

    static constexpr std::deque<Fruit>::size_type no_worm_index = -1;
    auto new_idx = collected_fruits.size() - 1;

    if (collected_fruits[new_idx].quality() != Quality::WORMY) return;

    auto start = (last_wormy_index == no_worm_index) ? 0 : last_wormy_index + 1;

    auto subrange = collected_fruits | std::views::drop(start) | std::views::take(new_idx - start);

    std::ranges::for_each(subrange, [](Fruit& f){
        if (f.quality() == Quality::HEALTHY && f.taste() == Taste::SWEET) {
            f.become_worm_infested();
        }
    });

    last_wormy_index = new_idx;
}

inline Picker& Picker::operator-=(Picker& other) {
    if (collected_fruits.empty()) return *this;

    Fruit stolen_fruit = collected_fruits.front();
    collected_fruits.pop_front();
    adjust_index_after_pop_front();
    other += stolen_fruit;

    return *this;
}

inline Picker& Picker::operator+=(Picker& other) {
    if (other.collected_fruits.empty()) return *this;

    Fruit stolen_fruit = other.collected_fruits.front();
    other.collected_fruits.pop_front();
    other.adjust_index_after_pop_front();

    *this += stolen_fruit;
    
    return *this;
}

inline auto Picker::operator<=>(const Picker& other) const {
    using CountFn = std::function<std::size_t(const Picker&)>;

    CountFn fns[] = {
        [](const Picker& p){ return p.count_quality(Quality::HEALTHY); },
        [](const Picker& p){ return p.count_taste(Taste::SWEET); },
        [](const Picker& p){ return p.count_size(Size::LARGE); },
        [](const Picker& p){ return p.count_size(Size::MEDIUM); },
        [](const Picker& p){ return p.count_size(Size::SMALL); },
        [](const Picker& p){ return p.count_fruits(); }
    };

    for (auto& fn : fns) {
        auto lhs = fn(*this);
        auto rhs = fn(other);
        if (lhs != rhs) return rhs <=> lhs;
    }

    return std::strong_ordering::equal;
}

inline void Picker::adjust_index_after_pop_front() {
    const auto npos = std::deque<Fruit>::size_type(-1);

    if (last_wormy_index == npos) {
        return;
    }

    last_wormy_index = (last_wormy_index == 0) ? npos : last_wormy_index - 1;
}

inline Ranking::Ranking(const Ranking& other) : pickers(other.pickers) {}

inline Ranking::Ranking(const std::initializer_list<Picker>& pickers_list) {
    pickers = std::vector<Picker>(pickers_list);
    std::stable_sort(pickers.begin(), pickers.end(), std::less<Picker>());
}

inline Ranking& Ranking::operator+=(const Picker& picker) {
    pickers.push_back(picker);
    std::stable_sort(pickers.begin(), pickers.end(), std::less<Picker>());
    return *this;
}

inline const Picker& Ranking::operator[](std::size_t index) const {
    if (pickers.empty()) {
        throw std::out_of_range("Ranking is empty");
    }
    if (index >= pickers.size()) {
        return pickers.back();
    }
    return pickers[index];
}

inline std::ostream& operator<<(std::ostream& os, const Ranking& ranking) {
    for (const auto& picker : ranking.pickers) {
        os << picker;
    }
    return os;
}

inline Ranking& Ranking::operator-=(const Picker& picker) {
    auto it = std::find(pickers.begin(), pickers.end(), picker);
    if (it != pickers.end()) pickers.erase(it);
    return *this;
}

inline Ranking& Ranking::operator+=(const Ranking& other) {
    std::vector<Picker> merged;
    merged.reserve(pickers.size() + other.pickers.size());

    auto it1 = pickers.begin();
    auto it2 = other.pickers.begin();

    while (it1 != pickers.end() && it2 != other.pickers.end()) {
        if (*it2 < *it1) {
            merged.push_back(*it2++);
        } else {
            merged.push_back(*it1++);
        }
    }

    merged.insert(merged.end(), it1, pickers.end());
    merged.insert(merged.end(), it2, other.pickers.end());
    
    pickers = std::move(merged);
    return *this;
}

inline Ranking Ranking::operator+(const Ranking& other) const {
    Ranking result(*this);
    result += other;
    return result;
}

constexpr Fruit YUMMY_ONE{Taste::SWEET, Size::LARGE, Quality::HEALTHY};
constexpr Fruit ROTTY_ONE{Taste::SOUR, Size::SMALL, Quality::ROTTEN};

#endif  // FRUIT_PICKING_H
