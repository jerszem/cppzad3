#ifndef FRUIT_PICKING_H
#define FRUIT_PICKING_H

#include <queue>
#include <vector>
#include <string>
#include <compare>
#include <iostream>
#include <deque>

enum class Taste {
    SWEET,
    SOUR
};

enum class Size {
    LARGE,
    MEDIUM,
    SMALL
};

enum class Quality {
    HEALTHY,
    ROTTEN,
    WORMY
};

class Fruit {
public:
    constexpr Fruit(Taste taste, Size size, Quality quality);
    constexpr Fruit(const Fruit& other);
    constexpr Fruit(std::tuple<Taste, Size, Quality>);
    void go_rotten();
    void become_worm_infested();
    constexpr Taste taste() const {return fruit_taste; }
    constexpr Size size() const {return fruit_size; }
    constexpr Quality quality() const {return fruit_quality; }
    constexpr bool operator==(const Fruit& other) const = default;

    // dodajemy konwersję do tuple jeśli w przykładzie używasz:
    // using fruit_tuple_t = std::tuple<Taste, Size, Quality>;
    inline operator std::tuple<Taste, Size, Quality>() const {
        return { taste(), size(), quality() }; // użyj właściwych getterów/pól
    }
    friend std::ostream& operator<<(std::ostream& os, const Fruit& fruit);

private:
    Taste fruit_taste;
    Size fruit_size;
    Quality fruit_quality;
};

class Picker {
public:
    Picker(const std::string&);
    Picker(std::string_view);
    // Picker(Picker&&) = default;

    // istniejący konstruktor
    Picker(const char* name = "Anonim");
    const std::string& get_name() const { return picker_name; }
    std::size_t count_fruits() const { return collected_fruits.size(); }
    std::size_t count_taste(Taste taste) const;
    std::size_t count_size(Size size) const;
    std::size_t count_quality(Quality quality) const;
    // We add a picked fruit to the picker. 
    // If it was healthy, it turns rotten if the last picked fruit was rotten;
    // if it was rotten and the previous one was healthy, the previous one turns rotten too.
    // If it was wormy, then all previously picked healthy and sweet fruits become wormy.
    Picker& operator+=(const Fruit& fruit);
    // We steal the first picked fruit from other picker
    Picker& operator+=(Picker& other);
    Picker& operator-=(Picker& other);
    bool operator==(const Picker& other) const;
    // We first compare by number of healthy fruits, then sweet fruits, then big, large, small, all fruits.
    auto operator<=>(const Picker& other) const;
    friend std::ostream& operator<<(std::ostream& os, const Picker& picker);
private:
    std::string picker_name;
    std::deque<Fruit> collected_fruits;
    // We remember the last wormy fruit picked to infect healthy sweet fruits
    std::deque<Fruit>::size_type last_wormy_index = std::deque<Fruit>::size_type(-1);
    void adjust_index_after_pop_front();
};

class Ranking {
public:
    Ranking() = default;
    Ranking(const Ranking& other);
    Ranking& operator=(const Ranking& other) = default; // <- dodane, aby pozbyć się ostrzeżenia
    // Initialize Ranking with a list of Pickers
    Ranking(const std::initializer_list<Picker>& pickers_list);
    std::size_t count_pickers() const { return pickers.size(); };
    friend std::ostream& operator<<(std::ostream& os, const Ranking& ranking);
    Ranking& operator+=(const Picker& picker);
    Ranking& operator-=(const Picker& picker);
    Ranking& operator+=(const Ranking& other);
    Ranking operator+(const Ranking& other) const;
    Picker& operator[](std::size_t index);
    const Picker& operator[](std::size_t index) const;
private:
    // Sorted by best picker first
    std::vector<Picker> pickers;
};

inline constexpr Fruit::Fruit(Taste taste, Size size, Quality quality)
    : fruit_taste(taste), fruit_size(size), fruit_quality(quality) {}

inline constexpr Fruit::Fruit(const Fruit& other)
    : fruit_taste(other.fruit_taste), fruit_size(other.fruit_size), fruit_quality(other.fruit_quality) {}

inline constexpr Fruit::Fruit(std::tuple<Taste, Size, Quality> tpl)
    : fruit_taste(std::get<0>(tpl)), fruit_size(std::get<1>(tpl)),
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
        case Taste::SWEET: os << "[słodki"; break;
        case Taste::SOUR: os << "[kwaśny"; break;
    }
    switch (fruit.size()) {
        case Size::LARGE: os << " duży"; break;
        case Size::MEDIUM: os << " średni"; break;
        case Size::SMALL: os << " mały"; break;
    }
    switch (fruit.quality()) {
        case Quality::HEALTHY: os << " zdrowy]"; break;
        case Quality::ROTTEN: os << " nadgniły]"; break;
        case Quality::WORMY: os << " robaczywy]"; break;
    }
    return os;
}

inline Picker::Picker(const char* name) {
    if (name != nullptr && name[0] != '\0') {
        picker_name = name;
    } else {
        picker_name = "Anonim";
    }
}

inline Picker::Picker(const std::string& name) {
    // delegujemy do konstrukcji wewnętrznej (lub ustawiamy pole)
    name == "" ? picker_name = "Anonim" : picker_name = name;
}

inline Picker::Picker(std::string_view name) {
    // bezpiecznie przekonwertuj do std::string (lub przypisz bezpośrednio jeśli pole to string_view)
    std::string tmp{name};
    *this = Picker(tmp.c_str());
}

inline std::size_t Picker::count_taste(Taste taste) const {
    std::size_t count = 0;
    for (const auto& fruit : collected_fruits) {
        if (fruit.taste() == taste) {
            ++count;
        }
    }
    return count;
}

inline std::size_t Picker::count_size(Size size) const {
    std::size_t count = 0;
    for (const auto& fruit : collected_fruits) {
        if (fruit.size() == size) {
            ++count;
        }
    }
    return count;
}

inline std::size_t Picker::count_quality(Quality quality) const {
    std::size_t count = 0;
    for (const auto& fruit : collected_fruits) {
        if (fruit.quality() == quality) {
            ++count;
        }
    }
    return count;
}

inline std::ostream& operator<<(std::ostream& os, const Picker& picker) {
    os << picker.picker_name << ":\n";
    for (const auto& fruit : picker.collected_fruits) {
        os << "\t" << fruit << "\n";
    }
    return os;
}

inline bool Picker::operator==(const Picker& other) const {
    if (picker_name != other.picker_name || collected_fruits.size() != other.collected_fruits.size()) {
        return false;
    }
    for (size_t i = 0; i < collected_fruits.size(); ++i) {
        if (!(collected_fruits[i] == other.collected_fruits[i])) {
            return false;
        }
    }
    return true;
}

inline Picker& Picker::operator+=(const Fruit& fruit) {
    collected_fruits.push_back(fruit);
    auto npos = std::deque<Fruit>::size_type(-1);
    auto new_idx = collected_fruits.size() - 1;

    // istniejąca logika gnicia między dwoma ostatnimi owocami
    if (collected_fruits.size() >= 2) {
        Fruit& last = collected_fruits.back();
        Fruit& second_last = collected_fruits[collected_fruits.size() - 2];
        if (last.quality() == Quality::ROTTEN && second_last.quality() == Quality::HEALTHY) {
            second_last.go_rotten();
        } else if (last.quality() == Quality::HEALTHY && second_last.quality() == Quality::ROTTEN) {
            last.go_rotten();
        }
    }

    // jeśli nowy owoc jest robaczywy — infekujemy tylko owoce dodane po poprzednim robaczym
    if (collected_fruits[new_idx].quality() == Quality::WORMY) {
        std::deque<Fruit>::size_type start = (last_wormy_index == npos) ? 0 : (last_wormy_index + 1);
        for (auto i = start; i < new_idx; ++i) {
            if (collected_fruits[i].quality() == Quality::HEALTHY &&
                collected_fruits[i].taste() == Taste::SWEET) {
                collected_fruits[i].become_worm_infested();
            }
        }
        last_wormy_index = new_idx;
    }

    return *this;
}

inline Picker& Picker::operator-=(Picker& other) {
    other += *this;
    return *this;
}

inline Picker& Picker::operator+=(Picker& other) {
    if (!other.collected_fruits.empty()) {
        Fruit stolen_fruit = other.collected_fruits.front();
        other.collected_fruits.pop_front();
        other.adjust_index_after_pop_front();
        *this += stolen_fruit;
    }
    return *this;
}

inline auto Picker::operator<=>(const Picker& other) const {
    // Who has more is smaller
    if (count_quality(Quality::HEALTHY) != other.count_quality(Quality::HEALTHY)) {
        return other.count_quality(Quality::HEALTHY) <=> count_quality(Quality::HEALTHY);
    }
    if (count_taste(Taste::SWEET) != other.count_taste(Taste::SWEET)) {
        return other.count_taste(Taste::SWEET) <=> count_taste(Taste::SWEET);
    }
    if (count_size(Size::LARGE) != other.count_size(Size::LARGE)) {
        return other.count_size(Size::LARGE) <=> count_size(Size::LARGE);
    }
    if (count_size(Size::MEDIUM) != other.count_size(Size::MEDIUM)) {
        return other.count_size(Size::MEDIUM) <=> count_size(Size::MEDIUM);
    }
    if (count_size(Size::SMALL) != other.count_size(Size::SMALL)) {
        return other.count_size(Size::SMALL) <=> count_size(Size::SMALL);
    }
    return other.count_fruits() <=> count_fruits();
}

inline void Picker::adjust_index_after_pop_front() {
    auto npos = std::deque<Fruit>::size_type(-1);
    if (last_wormy_index == npos) return;
    if (last_wormy_index == 0) last_wormy_index = npos;
    else --last_wormy_index;
}

inline Ranking::Ranking(const Ranking& other)
    : pickers(other.pickers) {}

inline Ranking::Ranking(const std::initializer_list<Picker>& pickers_list) {
    pickers = std::vector<Picker>(pickers_list);
    std::sort(pickers.begin(), pickers.end(), std::less<Picker>());
}

inline Ranking& Ranking::operator+=(const Picker& picker) {
    pickers.push_back(picker);
    std::sort(pickers.begin(), pickers.end(), std::less<Picker>());
    return *this;
}

inline Picker& Ranking::operator[](std::size_t index) {
    if (index >= pickers.size()) {
        throw std::out_of_range("Index out of range");
    }
    return pickers[index];
}

inline const Picker& Ranking::operator[](std::size_t index) const {
    if (index >= pickers.size()) {
        throw std::out_of_range("Index out of range");
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
    for (const auto& current : pickers) {
        if (current == picker) {
            pickers.erase(std::remove(pickers.begin(), pickers.end(), current), pickers.end());
        }
    }
    return *this;
}

inline Ranking& Ranking::operator+=(const Ranking& other) {
    std::vector<Picker> new_pickers;
    new_pickers.reserve(pickers.size() + other.pickers.size());
    std::merge(pickers.begin(), pickers.end(), other.pickers.begin(), other.pickers.end(), std::back_inserter(new_pickers));
    pickers = std::move(new_pickers);
    return *this;
}

inline Ranking Ranking::operator+(const Ranking& other) const {
    Ranking result(*this);
    result += other;
    return result;
}

constexpr Fruit YUMMY_ONE{Taste::SWEET, Size::LARGE, Quality::HEALTHY};
constexpr Fruit ROTTY_ONE{Taste::SOUR, Size::SMALL, Quality::ROTTEN};

#endif // FRUIT_PICKING_H
