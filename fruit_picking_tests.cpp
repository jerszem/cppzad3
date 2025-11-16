// Comprehensive tests for fruit_picking.h
// Build: g++ -std=c++23 -O2 -Wall -Wextra -Werror -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer fruit_picking_tests.cpp -o fruit_picking_tests

#include "fruit_picking.h"

#ifdef NDEBUG
  #undef NDEBUG
#endif

#include <cassert>
#include <concepts>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

using std::cout;
using std::string;
using std::string_view;
using std::tuple;
using fruit_tuple_t = tuple<Taste, Size, Quality>;

// ----------------- Helpers & Macros -----------------
#define FRUIT_ASSERTS(f, t, s, q) \
  do { \
    assert((f).taste() == (t)); \
    assert((f).size() == (s)); \
    assert((f).quality() == (q)); \
  } while (0)

#define PICKER_ASSERTS(p, all, swe, sou, lar, med, sma, hea, rot, wor) \
  do { \
    assert((p).count_fruits() == (all)); \
    assert((p).count_taste(Taste::SWEET) == (swe)); \
    assert((p).count_taste(Taste::SOUR) == (sou)); \
    assert((p).count_size(Size::LARGE) == (lar)); \
    assert((p).count_size(Size::MEDIUM) == (med)); \
    assert((p).count_size(Size::SMALL) == (sma)); \
    assert((p).count_quality(Quality::HEALTHY) == (hea)); \
    assert((p).count_quality(Quality::ROTTEN) == (rot)); \
    assert((p).count_quality(Quality::WORMY) == (wor)); \
  } while (0)



// ======================== TESTS1 ========================

// Concepts to detect availability of operators between different attribute types.
template <class A, class B>
concept HasLess = requires(A a, B b) { { a < b } -> std::convertible_to<bool>; };

template <class A, class B>
concept HasEq = requires(A a, B b) { { a == b } -> std::convertible_to<bool>; };

static void test_fruit_basics() {
  // Explicit construction, copying, moving, conversions, constexpr.
  Fruit f1{Taste::SWEET, Size::LARGE, Quality::HEALTHY};
  FRUIT_ASSERTS(f1, Taste::SWEET, Size::LARGE, Quality::HEALTHY);

  Fruit f2{f1}; // copy ctor
  FRUIT_ASSERTS(f2, Taste::SWEET, Size::LARGE, Quality::HEALTHY);

  Fruit f3{Taste::SOUR, Size::SMALL, Quality::ROTTEN};
  Fruit f4{std::move(f3)}; // move ctor
  FRUIT_ASSERTS(f4, Taste::SOUR, Size::SMALL, Quality::ROTTEN);

  // Tuple conversions must be explicit in both directions.
  fruit_tuple_t ft1{Taste::SOUR, Size::MEDIUM, Quality::WORMY};
  Fruit f5{ft1}; // explicit from tuple
  FRUIT_ASSERTS(f5, Taste::SOUR, Size::MEDIUM, Quality::WORMY);

  fruit_tuple_t ft2 = static_cast<fruit_tuple_t>(f1); // explicit to tuple
  auto const& [t, s, q] = ft2;
  assert(t == Taste::SWEET && s == Size::LARGE && q == Quality::HEALTHY);

  // constexpr usage for non-mutating methods
  constexpr Fruit cf{Taste::SWEET, Size::SMALL, Quality::HEALTHY};
  static_assert(cf.taste() == Taste::SWEET);
  static_assert(cf.size() == Size::SMALL);
  static_assert(cf.quality() == Quality::HEALTHY);

  // Global compile-time constants
  static_assert(YUMMY_ONE.taste() == Taste::SWEET);
  static_assert(YUMMY_ONE.size() == Size::LARGE);
  static_assert(YUMMY_ONE.quality() == Quality::HEALTHY);

  static_assert(ROTTY_ONE.taste() == Taste::SOUR);
  static_assert(ROTTY_ONE.size() == Size::SMALL);
  static_assert(ROTTY_ONE.quality() == Quality::ROTTEN);

  // Mutations
  Fruit m{Taste::SWEET, Size::LARGE, Quality::HEALTHY};
  m.go_rotten();
  FRUIT_ASSERTS(m, Taste::SWEET, Size::LARGE, Quality::ROTTEN);
  Fruit w{Taste::SOUR, Size::MEDIUM, Quality::HEALTHY};
  w.become_worm_infested();
  FRUIT_ASSERTS(w, Taste::SOUR, Size::MEDIUM, Quality::WORMY);
}

static void test_fruit_type_safety_and_stream() {
  // No default construction.
  static_assert(!std::is_default_constructible_v<Fruit>);

  // Tuple/Fruit conversions should be explicit.
  static_assert(!std::is_convertible_v<fruit_tuple_t, Fruit>);
  static_assert(!std::is_convertible_v<Fruit, fruit_tuple_t>);

  // Cross-attribute implicit conversion and comparison should not exist.
  static_assert(!std::is_convertible_v<Taste, Size>);
  static_assert(!std::is_convertible_v<Size, Taste>);
  static_assert(!std::is_convertible_v<Taste, Quality>);
  static_assert(!std::is_convertible_v<Quality, Taste>);
  static_assert(!std::is_convertible_v<Size, Quality>);
  static_assert(!std::is_convertible_v<Quality, Size>);

  static_assert(!HasLess<Taste, Size>);
  static_assert(!HasLess<Size, Taste>);
  static_assert(!HasLess<Taste, Quality>);
  static_assert(!HasLess<Quality, Taste>);
  static_assert(!HasLess<Size, Quality>);
  static_assert(!HasLess<Quality, Size>);

  // Equality should be only within the same attribute type.
  static_assert(!HasEq<Taste, Size>);
  static_assert(!HasEq<Taste, Quality>);
  static_assert(!HasEq<Size, Quality>);

  // Stream format smoke test.
  std::ostringstream oss;
  oss << Fruit{Taste::SWEET, Size::MEDIUM, Quality::HEALTHY};
  const std::string out = oss.str();
  assert(!out.empty());
  // Expect square brackets and attribute tokens.
  assert(out.front() == '[' && out.back() == ']');
  assert(out.find("słodki") != std::string::npos || out.find("kwaśny") != std::string::npos);
  assert(out.find("duży") != std::string::npos || out.find("średni") != std::string::npos || out.find("mały") != std::string::npos);
  assert(out.find("zdrowy") != std::string::npos || out.find("nadgniły") != std::string::npos || out.find("robaczywy") != std::string::npos);
}

static void test_picker_basics_and_transforms() {
  // Default name and empty string -> "Anonim".
  Picker p0{};
  assert(p0.get_name() == "Anonim");
  Picker p0b{""};
  assert(p0b.get_name() == "Anonim");

  // Adding fruits and checking counts + transformation rules.
  Picker p1{"Tester"};
  PICKER_ASSERTS(p1, 0, 0, 0, 0, 0, 0, 0, 0, 0);

  // 1) Healthy after Rotten -> new becomes Rotten.
  p1 += Fruit{Taste::SWEET, Size::LARGE, Quality::ROTTEN};
  PICKER_ASSERTS(p1, 1, 1, 0, 1, 0, 0, 0, 1, 0);
  p1 += Fruit{Taste::SOUR, Size::SMALL, Quality::HEALTHY}; // should become Rotten
  PICKER_ASSERTS(p1, 2, 1, 1, 1, 0, 1, 0, 2, 0);

  // 2) Rotten after Healthy -> previous Healthy becomes Rotten.
  Picker p2{"P2"};
  p2 += Fruit{Taste::SWEET, Size::SMALL, Quality::HEALTHY}; // H
  PICKER_ASSERTS(p2, 1, 1, 0, 0, 0, 1, 1, 0, 0);
  p2 += Fruit{Taste::SOUR, Size::LARGE, Quality::ROTTEN};   // makes first Rotten
  PICKER_ASSERTS(p2, 2, 1, 1, 1, 0, 1, 0, 2, 0);

  // 3) Wormy -> all previously Healthy & Sweet become Wormy.
  Picker p3{"P3"};
  p3 += Fruit{Taste::SWEET, Size::LARGE, Quality::HEALTHY}; // HS
  p3 += Fruit{Taste::SOUR, Size::MEDIUM, Quality::HEALTHY}; // H (SOUR)
  p3 += Fruit{Taste::SWEET, Size::SMALL, Quality::ROTTEN};  // R
  p3 += Fruit{Taste::SWEET, Size::MEDIUM, Quality::WORMY};  // trigger -> convert Healthy & SWEET to WORMY
  // Now: [HS -> W, H(sour stays H), R, W]
  PICKER_ASSERTS(p3, 4, /*sweet*/3, /*sour*/1, /*L*/1, /*M*/2, /*S*/1, /*H*/0, /*R*/2, /*W*/2);
}

static void test_picker_steal_and_give() {
  Picker a{"A"}, b{"B"};
  a += YUMMY_ONE;            // [Y]
  a += Fruit{Taste::SOUR, Size::SMALL, Quality::ROTTEN}; // [Y,R]
  b += Fruit{Taste::SOUR, Size::LARGE, Quality::HEALTHY}; // [H]

  // A steals earliest from B via "+="
  a += b; // steal b.front() -> append to a's end
  // TODO -- do stolen fruit become rotten?
  PICKER_ASSERTS(a, 3, /*sweet*/1, /*sour*/2, /*L*/2, /*M*/0, /*S*/1, /*H*/0, /*R*/3, /*W*/0 /* keep calc simple */);
  // Above W calc not trivial; do a safer invariant instead:
  assert(a.count_quality(Quality::HEALTHY) + a.count_quality(Quality::ROTTEN) + a.count_quality(Quality::WORMY) == a.count_fruits());
  PICKER_ASSERTS(b, 0, 0, 0, 0, 0, 0, 0, 0, 0);

  // Give from front via "-="
  Picker c{"C"};
  c += Fruit{Taste::SWEET, Size::SMALL, Quality::HEALTHY};
  c += Fruit{Taste::SWEET, Size::SMALL, Quality::HEALTHY};
  b -= c; // b gives first to c; but b is empty -> no change
  PICKER_ASSERTS(b, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  c -= b; // c gives first to b
  PICKER_ASSERTS(c, 1, 1, 0, 0, 0, 1, /*H*/1, 0, 0);
  PICKER_ASSERTS(b, 1, 1, 0, 0, 0, 1, /*H*/1, 0, 0);
}

static void test_picker_equality_and_ordering() {
  // Equality: same name and identical sequence -> equal.
  Picker p1{"Arnold"};
  p1 += YUMMY_ONE;
  p1 += ROTTY_ONE;
  Picker p2{string{"Arnold"}};
  p2 += YUMMY_ONE;
  p2 += ROTTY_ONE;
  assert(p1 == p2);

  // Different order -> not equal even if counts match.
  Picker p3{"Arnold"};
  p3 += ROTTY_ONE;
  p3 += YUMMY_ONE;
  assert(p1 != p3);

  // Strict weak ordering (spaceship): craft tie-breakers step by step.
  Picker a{"A"}, b{"B"};
  // Start identical
  a += YUMMY_ONE;              // +1 healthy, +1 sweet, +1 large
  b += YUMMY_ONE;
  assert((a <=> b) == 0);

  // a: turn last into Rotten by adding Rotten after Healthy -> previous Healthy becomes Rotten
  a += Fruit{Taste::SOUR, Size::LARGE, Quality::ROTTEN};
  // Now: a has 0 healthy vs b has 1 healthy -> b ranks higher
  assert((a <=> b) > 0);

  b += Fruit{Taste::SOUR, Size::SMALL, Quality::ROTTEN};
  assert((a <=> b) < 0);

  // Next tie-breaker: SWEET count.
  a += Fruit{Taste::SWEET, Size::SMALL, Quality::ROTTEN}; // doesn't change healthy; +sweet

  assert((a <=> b) < 0);

  // If SWEET equal, check LARGE, then MEDIUM, then SMALL, then total.
  // Force equality of sweet, then test size priority by giving b LARGE, a MEDIUM.
  b += Fruit{Taste::SWEET, Size::LARGE, Quality::ROTTEN};
  a += Fruit{Taste::SWEET, Size::MEDIUM, Quality::ROTTEN};
  assert((a <=> b) < 0);
}

static void test_ranking_basics() {
  Picker p1{"Arnold"};
  p1 += YUMMY_ONE;
  p1 += ROTTY_ONE;
  Picker p2{"Sylvester"};
  p2 += YUMMY_ONE;
  p2 += YUMMY_ONE;
  p2 += ROTTY_ONE;
  Picker p3{}; // Anonim
  p3 += Fruit{Taste::SOUR, Size::MEDIUM, Quality::WORMY};

  Ranking r1{p3, p2, p1};
  assert(r1.count_pickers() == 3);
  // Expected order: p2 (best), p1, p3
  assert(r1[0] == p2);
  assert(r1[1] == p1);
  assert(r1[2] == p3);

  // Adding and removing
  Ranking r2{};
  assert(r2.count_pickers() == 0);
  r2 += p1;
  assert(r2.count_pickers() == 1);
  Picker p4{"Chuck"};
  p4 += Fruit{Taste::SOUR, Size::MEDIUM, Quality::WORMY};
  r2 += p4;
  r2 += p3;
  r2 += p4; // allow duplicates
  assert(r2.count_pickers() == 4);
  // Remove highest-noted equal to p3 (if present)
  r2 -= p3;
  assert(r2.count_pickers() == 3);
  r2 -= p3; // no-op (no equal picker left)
  assert(r2.count_pickers() == 3);

  // Printing preserves order (names appear in rank order)
  std::ostringstream oss;
  oss << r1;
  std::string s = oss.str();
  assert(s.find("RANKING") == std::string::npos); // we didn't print headers here
  auto pos2 = s.find("Sylvester");
  auto pos1 = s.find("Arnold");
  auto pos3 = s.find("Anonim");
  assert(pos2 != std::string::npos && pos1 != std::string::npos && pos3 != std::string::npos);
  assert(pos2 < pos1 && pos1 < pos3);
}

static void test_ranking_merges_and_indexing() {
  Picker a{"A"}; a += YUMMY_ONE;
  Picker b{"B"}; b += ROTTY_ONE; // worse
  Picker c{"C"}; c += YUMMY_ONE; c += ROTTY_ONE; // middle

  Ranking r1{a, b};
  Ranking r2{c};

  Ranking r3{};
  r3 += r1;
  assert(r3.count_pickers() == 2);
  r3 += r2;
  assert(r3.count_pickers() == 3);

  // operator+ creates a new ranking with combined entries
  Ranking r4 = r1 + r2;
  assert(r4.count_pickers() == 3);

  // operator[]: out-of-range -> last element (const reference!)
  auto const& last = r4[9999];
  auto const& supposed_last = r4[r4.count_pickers() - 1];
  assert(&last == &supposed_last);

  using RefT = std::remove_reference_t<decltype(r4[0])>;
  static_assert(std::is_const_v<RefT>, "operator[] must not allow modification");
}

static void test_copy_move_semantics() {
  Picker p{"Mover"};
  p += YUMMY_ONE;

  Ranking rA{p};
  Ranking rB{rA}; // copy-construct
  assert(rA.count_pickers() == rB.count_pickers());
  assert(rA[0] == rB[0]);

  Ranking rC{std::move(rA)}; // move-construct
  assert(rC.count_pickers() == rB.count_pickers());

  Ranking rD{};
  rD = rC; // copy-assign
  assert(rD.count_pickers() == rC.count_pickers());

  Ranking rE{};
  rE = std::move(rC); // move-assign
  assert(rE.count_pickers() == rB.count_pickers());
}

static void test_random_invariants() {
  std::mt19937_64 rng(123456789ULL);
  std::uniform_int_distribution<int> t01(0, 1);      // Taste: 0..1
  std::uniform_int_distribution<int> s02(0, 2);      // Size: 0..2
  std::uniform_int_distribution<int> q02(0, 2);      // Quality: 0..2
  std::uniform_int_distribution<int> who(0, 2);      // which picker

  std::vector<Picker> pickers;
  pickers.emplace_back("P0");
  pickers.emplace_back("P1");
  pickers.emplace_back("P2");

  for (int i = 0; i < 1000; ++i) {
    int w = who(rng);
    Taste t = t01(rng) == 0 ? Taste::SWEET : Taste::SOUR;
    Size s = static_cast<Size>(s02(rng));
    Quality q = static_cast<Quality>(q02(rng));
    pickers[w] += Fruit{t, s, q};
  }

  // Invariants for each picker.
  for (auto const& p : pickers) {
    using C = decltype(p.count_fruits());
    const C tastes    = p.count_taste(Taste::SWEET) + p.count_taste(Taste::SOUR);
    const C sizes     = p.count_size(Size::LARGE) + p.count_size(Size::MEDIUM) + p.count_size(Size::SMALL);
    const C qualities = p.count_quality(Quality::HEALTHY) + p.count_quality(Quality::ROTTEN) + p.count_quality(Quality::WORMY);
    const C total     = p.count_fruits();

    assert(tastes == total);
    assert(sizes == total);
    assert(qualities == total);

  }

  // Build a ranking and check that indexing never goes out-of-bounds and returns const refs.
  Ranking r;
  r += pickers[0];
  r += pickers[1];
  r += pickers[2];
  for (int i = -10; i < 100; ++i) {
    auto const& ref = r[static_cast<size_t>(i)];
    (void)ref; // just ensure it's a const reference and accessible
  }
}




// ======================== TESTS2 ========================


#include "fruit_picking.h"

#ifdef NDEBUG
    #undef NDEBUG
#endif

#include <iostream>
#include <cassert>
#include <string>
#include <string_view>
#include <tuple>
#include <sstream>
#include <vector>
#include <utility>

// Makro pomocnicze do zliczania atrybutów owoców u zbieracza
#define ASSERT_PICKER_COUNTS(p, all, swe, sou, lar, med, sma, hea, rot, wor) \
do { \
    assert(p.count_fruits() == all); \
    assert(p.count_taste(Taste::SWEET) == swe); \
    assert(p.count_taste(Taste::SOUR) == sou); \
    assert(p.count_size(Size::LARGE) == lar); \
    assert(p.count_size(Size::MEDIUM) == med); \
    assert(p.count_size(Size::SMALL) == sma); \
    assert(p.count_quality(Quality::HEALTHY) == hea); \
    assert(p.count_quality(Quality::ROTTEN) == rot); \
    assert(p.count_quality(Quality::WORMY) == wor); \
} while(0)

// Pomocnicza funkcja do tworzenia tymczasowych obiektów
Picker create_picker(std::string_view name) {
    Picker p{name};
    p += YUMMY_ONE;
    return p;
}

Ranking create_ranking() {
    return Ranking{Picker{"Tmp1"}, Picker{"Tmp2"}};
}

namespace MaliciousTests {

void test_fruit_class() {
    std::cout << "Rozpoczynam testy klasy Fruit...\n";

    // Testy constexpr i constinit
    static_assert(YUMMY_ONE.taste() == Taste::SWEET);
    static_assert(YUMMY_ONE.size() == Size::LARGE);
    static_assert(YUMMY_ONE.quality() == Quality::HEALTHY);
    static_assert(ROTTY_ONE.taste() == Taste::SOUR);
    static_assert(ROTTY_ONE.size() == Size::SMALL);
    static_assert(ROTTY_ONE.quality() == Quality::ROTTEN);

    constexpr Fruit const_fruit{Taste::SOUR, Size::MEDIUM, Quality::WORMY};
    static_assert(const_fruit.taste() == Taste::SOUR);
    static_assert(const_fruit.size() == Size::MEDIUM);
    static_assert(const_fruit.quality() == Quality::WORMY);

    // Testy konstruktorów i konwersji
    Fruit f1(Taste::SWEET, Size::LARGE, Quality::HEALTHY);
    assert(f1 == YUMMY_ONE);

    Fruit f2(f1); // Kopiowanie
    assert(f1 == f2);

    Fruit f3(std::move(f2)); // Przenoszenie
    assert(f1 == f3);

    using FruitTuple = std::tuple<Taste, Size, Quality>;
    FruitTuple tuple_from_f1 = static_cast<FruitTuple>(f1);
    assert(std::get<0>(tuple_from_f1) == Taste::SWEET);

    Fruit f4{tuple_from_f1};
    assert(f1 == f4);

    // Testy przypisania
    Fruit f5{Taste::SOUR, Size::SMALL, Quality::ROTTEN};
    assert(f5 == ROTTY_ONE);
    f5 = f1; // Przypisanie kopiujące
    assert(f5 == f1);
    f5 = std::move(f3); // Przypisanie przenoszące
    assert(f5 == f1);
    f5 = f5; // Samoprzypisanie
    assert(f5 == f1);
    //TODO 
    // f5 = std::move(f5); // Samoprzypisanie przenoszące
    // assert(f5 == f1);

    // Testy modyfikacji stanu
    Fruit healthy_fruit = YUMMY_ONE;
    healthy_fruit.go_rotten();
    assert(healthy_fruit.quality() == Quality::ROTTEN);
    healthy_fruit.go_rotten(); // Powinno pozostać ROTTEN
    assert(healthy_fruit.quality() == Quality::ROTTEN);
    healthy_fruit.become_worm_infested(); // Nie powinno się zmienić z ROTTEN na WORMY
    assert(healthy_fruit.quality() == Quality::ROTTEN);

    healthy_fruit = YUMMY_ONE;
    healthy_fruit.become_worm_infested();
    assert(healthy_fruit.quality() == Quality::WORMY);
    healthy_fruit.become_worm_infested(); // Powinno pozostać WORMY
    assert(healthy_fruit.quality() == Quality::WORMY);
    healthy_fruit.go_rotten(); // Nie powinno się zmienić z WORMY na ROTTEN
    assert(healthy_fruit.quality() == Quality::WORMY);
    
    // Test operatora wyjścia
    std::stringstream ss;
    ss << YUMMY_ONE;
    assert(ss.str() == "[słodki duży zdrowy]");

    std::cout << "Testy klasy Fruit zakonczone sukcesem.\n";
}


void test_picker_class() {
    std::cout << "Rozpoczynam testy klasy Picker...\n";

    // Testy konstruktorów
    Picker p_anon{};
    assert(p_anon.get_name() == "Anonim");
    Picker p_anon2{""};
    assert(p_anon2.get_name() == "Anonim");

    Picker p1{"Zlosliwiec"};
    assert(p1.get_name() == "Zlosliwiec");
    ASSERT_PICKER_COUNTS(p1, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    // Testy operator+= (Fruit) - szczegółowe scenariusze
    // 1. HEALTHY po ROTTEN -> nowy owoc staje się ROTTEN
    p1 += ROTTY_ONE; // [SOUR SMALL ROTTEN]
    p1 += YUMMY_ONE; // Miał być [SWEET LARGE HEALTHY], ale staje się ROTTEN
    ASSERT_PICKER_COUNTS(p1, 2, 1, 1, 1, 0, 1, 0, 2, 0);

    // 2. ROTTEN po HEALTHY -> poprzedni owoc staje się ROTTEN
    Picker p2{"Dobromir"};
    p2 += YUMMY_ONE; // [SWEET LARGE HEALTHY]
    p2 += ROTTY_ONE; // [SOUR SMALL ROTTEN] -> poprzedni staje się ROTTEN
    ASSERT_PICKER_COUNTS(p2, 2, 1, 1, 1, 0, 1, 0, 2, 0);

    // 3. WORMY -> wszystkie poprzednie (HEALTHY i SWEET) stają się WORMY
    Picker p3{"Pechowiec"};
    p3 += Fruit{Taste::SWEET, Size::LARGE, Quality::HEALTHY}; // Stanie się WORMY
    p3 += Fruit{Taste::SOUR, Size::LARGE, Quality::HEALTHY}; // Stanie się ROTTEN
    p3 += Fruit{Taste::SWEET, Size::SMALL, Quality::ROTTEN}; // Pozostanie ROTTEN
    p3 += Fruit{Taste::SWEET, Size::MEDIUM, Quality::HEALTHY}; // Stanie się ROTTEN
    p3 += Fruit{Taste::SWEET, Size::LARGE, Quality::WORMY};
    ASSERT_PICKER_COUNTS(p3, 5, 4, 1, 3, 1, 1, 0, 3, 2);



    // 3. WORMY -> wszystkie poprzednie (HEALTHY i SWEET) stają się WORMY
    Picker p100{"Student_Debil"};
    p100 += Fruit{Taste::SWEET, Size::LARGE, Quality::HEALTHY}; // Stanie się WORMY
    p100 += Fruit{Taste::SOUR, Size::LARGE, Quality::HEALTHY}; // Pozostanie HEALTHY
    p100 += Fruit{Taste::SWEET, Size::SMALL, Quality::HEALTHY}; // Stanie się WORMY
    p100 += Fruit{Taste::SOUR, Size::MEDIUM, Quality::HEALTHY}; // Pozostanie HEALTHY
    p100 += Fruit{Taste::SOUR, Size::LARGE, Quality::WORMY};
    ASSERT_PICKER_COUNTS(p100, 5, 2, 3, 3, 1, 1, 2, 0, 3);
    
    
    // Testy przekazywania owoców między zbieraczami
    Picker p4{"Darczyńca"};
    p4 += YUMMY_ONE;
    p4 += ROTTY_ONE;
    Picker p5{"Biorca"};
    
    p4 -= p5; // p4 oddaje YUMMY_ONE do p5
    assert(p4.count_fruits() == 1);
    assert(p5.count_fruits() == 1);
    ASSERT_PICKER_COUNTS(p5, 1, 1, 0, 1, 0, 0, 0, 1, 0);

    p5 += p4; // p5 zabiera ROTTY_ONE od p4
    assert(p4.count_fruits() == 0);
    assert(p5.count_fruits() == 2);
    // Sprawdzenie, czy reguły psucia się NIE aplikują się przy przekazywaniu
    // Ostatni owoc w p5 to YUMMY_ONE (ROTTEN), dodany został ROTTY_ONE.
    ASSERT_PICKER_COUNTS(p5, 2, 1, 1, 1, 0, 1, 0, 2, 0);

    // Testy operacji na samym sobie
    p5 += p5;
    assert(p5.count_fruits() == 2);
    p5 -= p5;
    assert(p5.count_fruits() == 2);
    
    // Testy operatorów porównania
    Picker CmpA{"A"}, CmpB{"B"};
    // 1. Kryterium HEALTHY
    CmpA += YUMMY_ONE; // 1 HEALTHY
    assert((CmpA <=> CmpB) < 0);
    // 2. Kryterium SWEET
    CmpB += Fruit{Taste::SOUR, Size::LARGE, Quality::HEALTHY}; // 1 HEALTHY
    assert(CmpA.count_quality(Quality::HEALTHY) == CmpB.count_quality(Quality::HEALTHY));
    assert((CmpA <=> CmpB) < 0); // CmpA ma 1 SWEET, CmpB ma 0
    // ... i tak dalej dla wszystkich kryteriów
    
    // Test remisu w <=>
    Picker Tie1{"TIE1"}, Tie2{"TIE2"};
    Tie1 += YUMMY_ONE;
    Tie2 += YUMMY_ONE;
    assert((Tie1 <=> Tie2) == 0);

    // Test operatora ==
    Picker Eq1{"Eq"}, Eq2{"Eq"};
    Eq1 += YUMMY_ONE;
    Eq2 += YUMMY_ONE;
    assert(Eq1 == Eq2);
    Eq2 += ROTTY_ONE;
    assert(Eq1 != Eq2);
    Picker Eq3{"InneImie"};
    Eq3 += YUMMY_ONE;
    assert(Eq1 != Eq3);

    // Testy operatora wyjścia
    std::stringstream ss;
    ss << p2;
    std::string expected_out = "Dobromir:\n\t[słodki duży nadgniły]\n\t[kwaśny mały nadgniły]";
    assert(ss.str() == expected_out);
    
    ss.str("");
    ss << p_anon;
    assert(ss.str() == "Anonim:");
    
    std::cout << "Testy klasy Picker zakonczone sukcesem.\n";
}


void test_ranking_class() {
    std::cout << "Rozpoczynam testy klasy Ranking...\n";

    Picker arnold{"Arnold"};
    arnold += YUMMY_ONE; // 1H, 1S, 1L
    Picker sylvester{"Sylvester"};
    sylvester += YUMMY_ONE;
    sylvester += YUMMY_ONE; // 2H, 2S, 2L
    Picker chuck{"Chuck"};
    chuck += ROTTY_ONE; // 0H, 0S, 0L
    Picker anonim{}; // 0H, 0S, 0L

    // Test konstruktora z listy inicjującej i sortowania
    Ranking r1{arnold, chuck, sylvester};
    assert(r1.count_pickers() == 3);
    assert(r1[0].get_name() == "Sylvester");
    assert(r1[1].get_name() == "Arnold");
    assert(r1[2].get_name() == "Chuck");

    // Test operatora [] poza zakresem
    assert(r1[3].get_name() == "Chuck");
    assert(r1[999].get_name() == "Chuck");
    const Ranking& const_r1 = r1;
    assert(const_r1[5].get_name() == "Chuck");

    // Test operatora [] na pustym rankingu (zachowanie niezdefiniowane, ale sprawdzamy czy nie crashuje)
    Ranking empty_r;
    try {
        [[maybe_unused]] Picker p = empty_r[0];
        // Jeśli tu dotrze, to znaczy że program się nie zawiesił
    } catch (...) {
        std::cout<<"EXCEPTION. OK\n";
        // Złapanie wyjątku też jest ok
    }

    // Test dodawania zbieraczy i tie-breaking
    Ranking r2;
    Picker chuck_clone{"Chuck"}; // Taki sam jak chuck, ale dodany później
    chuck_clone += ROTTY_ONE;
    r2 += chuck;
    r2 += anonim;
    r2 += chuck_clone; // Powinien być po oryginalnym 'chucku'
    assert(r2.count_pickers() == 3);
    assert(r2[0].get_name() == "Chuck" && r2[0] == chuck);
    assert(r2[1].get_name() == "Chuck" && r2[1] == chuck_clone);
    assert(r2[2].get_name() == "Anonim");

    // Test usuwania
    Picker another_chuck_clone{"Chuck"};
    another_chuck_clone += ROTTY_ONE;
    r2 -= another_chuck_clone; // Usuwa pierwszego napotkanego 'chucka'
    assert(r2.count_pickers() == 2);
    assert(r2[0].get_name() == "Chuck");
    assert(r2[1].get_name() == "Anonim");
    
    // Próba usunięcia kogoś, kogo nie ma
    r2 -= sylvester;
    assert(r2.count_pickers() == 2);

    // Testy łączenia rankingów
    Ranking r3 = r1 + r2;
    assert(r3.count_pickers() == 5);
    assert(r3[0].get_name() == "Sylvester");
    assert(r3[1].get_name() == "Arnold");
    assert(r3[2].get_name() == "Chuck");
    assert(r3[3].get_name() == "Chuck");
    assert(r3[4].get_name() == "Anonim");

    Ranking r4;

    r4 += r1;

    assert(r4.count_pickers() == 3);
    // Test dodawania rankingu do samego siebie

    r4 += r4; // Duplikuje zawartość
    assert(r4.count_pickers() == 6);
    assert(r4[0].get_name() == "Sylvester" && r4[1].get_name() == "Sylvester");
    assert(r4[2].get_name() == "Arnold" && r4[3].get_name() == "Arnold");
    assert(r4[4].get_name() == "Chuck" && r4[5].get_name() == "Chuck");

    Ranking r_move_test = create_ranking();
    r_move_test += std::move(r_move_test); // Powinno być no-op
    assert(r_move_test.count_pickers() == 2);


    // Testy semantyki przenoszenia
    r4 += create_picker("Temp"); // z r-wartości
    assert(r4.count_pickers() == 7);
    
    Ranking r5 = create_ranking();
    Ranking r6 = std::move(r5);
    assert(r6.count_pickers() == 2);
    // Stan r5 jest teraz "ważny, ale nieokreślony". Najczęściej pusty.
    assert(r5.count_pickers() == 0); // Nie jest to gwarantowane, ale częste

    // Test operatora wyjścia
    std::stringstream ss;
    ss << r1;
    std::string expected_out = "Sylvester:\n\t[słodki duży zdrowy]\n\t[słodki duży zdrowy]\nArnold:\n\t[słodki duży zdrowy]\nChuck:\n\t[kwaśny mały nadgniły]\n";
    assert(ss.str() == expected_out);

    ss.str("");
    ss << empty_r;
    assert(ss.str() == "");
    
    std::cout << "Testy klasy Ranking zakonczone sukcesem.\n";
}


void test_language_features() {
    std::cout << "Rozpoczynam testy zaawansowanych cech jezyka C++...\n";

    // Testy const-correctness
    const Fruit const_fruit{Taste::SWEET, Size::MEDIUM, Quality::HEALTHY};
    assert(const_fruit.taste() == Taste::SWEET);

    Picker p{"Tester"};
    p += const_fruit;
    const Picker const_picker = p;
    assert(const_picker.get_name() == "Tester");
    assert(const_picker.count_fruits() == 1);
    assert(const_picker.count_quality(Quality::HEALTHY) == 1);

    Ranking r{const_picker};
    const Ranking const_ranking = r;
    assert(const_ranking.count_pickers() == 1);
    assert(const_ranking[0].get_name() == "Tester");

    // Poniższe linie kodu nie powinny się kompilować, jeśli const jest użyte poprawnie.
    // const_fruit.go_rotten();
    // const_picker += const_fruit;
    // const_ranking += const_picker;
    // const_ranking[0] = p; // Jeśli operator[] zwraca referencję do const, to się nie skompiluje

    // Testy 'explicit'
    // Poniższe linie nie powinny się kompilować z powodu 'explicit'
    // Fruit f = {Taste::SWEET, Size::LARGE, Quality::HEALTHY};
    // Fruit f_tuple = std::make_tuple(Taste::SWEET, Size::LARGE, Quality::HEALTHY);
    // void take_fruit(Fruit);
    // take_fruit({Taste::SWEET, Size::LARGE, Quality::HEALTHY});    



    std::cout << "Testy zaawansowanych cech jezyka C++ zakonczone sukcesem.\n";
}

// ========= TESTY STATIC ASSERT ===========


using ElemRef = decltype(std::declval<const Ranking&>()[0]);
static_assert(std::is_same_v<ElemRef, const Picker&>);
static_assert(!std::is_assignable_v<ElemRef, Picker>);


static_assert(!std::is_convertible_v<fruit_tuple_t, Fruit>);   // brak konwersji niejawnej
static_assert( std::is_constructible_v<Fruit, fruit_tuple_t>); // ale można jawnie zbudować

static_assert(!std::is_convertible_v<Fruit, fruit_tuple_t>);   // brak konwersji niejawnej
// direct-list init i static_cast działają:
static_assert( std::is_constructible_v<fruit_tuple_t, Taste, Size, Quality>); 

// We assert that the conversion is NOT possible.
static_assert(!std::is_convertible_v<std::tuple<Taste, Size, Quality>, Fruit>,
              "Fruit should not be implicitly constructible from a tuple.");

// The same for the other direction.
static_assert(!std::is_convertible_v<Fruit, std::tuple<Taste, Size, Quality>>,
              "Tuple should not be implicitly constructible from a Fruit.");

  
// --- SFINAE Helper for testing const-correctness ---
// This template will resolve to true_type if the expression inside decltype is valid,
// and false_type otherwise.
template <typename, typename = void>
struct is_expression_valid : std::false_type {};

template <typename T>
struct is_expression_valid<T, std::void_t<T>> : std::true_type {};


// Helper to check `const_fruit.go_rotten()`
template <typename F>
constexpr bool can_call_go_rotten = requires(F f) {
    f.go_rotten();
};
// Helper to check `const_picker += const_fruit;`
template <typename P, typename F>
constexpr bool can_add_fruit_to_picker = requires(P p, F f) {
    p += f;
};

    // Helper to check `const_ranking += const_picker;`
template <typename R, typename P>
constexpr bool can_add_picker_to_ranking = requires(R r, P p) {
    r += p;
};
    // This helper checks if a type can be initialized with `Type t = {args...};`
    // It works by simulating a function call, which uses copy-initialization.
    template <typename... Args>
    constexpr bool is_copy_list_initializable = requires(Args... args) {
        [](Fruit){}({std::declval<Args>()...});
    };
void test_compile_time_properties_with_static_assert() {

  // --- Part 1: Testing 'explicit' (this part was already correct) ---
  static_assert(!std::is_convertible_v<std::tuple<Taste, Size, Quality>, Fruit>,
                "FAIL: Fruit should NOT be implicitly convertible from a tuple.");

  static_assert(!std::is_convertible_v<Fruit, std::tuple<Taste, Size, Quality>>,
                "FAIL: std::tuple should NOT be implicitly convertible from a Fruit.");

  // --- Part 2: Testing 'const'-correctness (REWRITTEN WITH `requires`) ---

  // We define small helpers using `requires` to check if an expression is valid.
  // This is the modern replacement for the SFINAE `is_expression_valid` helper.

  static_assert(!can_call_go_rotten<const Fruit>,
                "FAIL: Should not be able to call go_rotten() on a const Fruit.");

  static_assert(!can_add_fruit_to_picker<const Picker&, const Fruit&>,
                "FAIL: Should not be able to call operator+= on a const Picker.");

  static_assert(!can_add_picker_to_ranking<const Ranking&, const Picker&>,
                "FAIL: Should not be able to call operator+= on a const Ranking.");

  // Helper to check `const_ranking[0] = p;`
  // For this specific check, `std::is_assignable_v` is still the best tool,
  // as it's designed to check for assignment. It works correctly without issues.
  using ConstRankingIndexResult = decltype(std::declval<const Ranking&>()[0]);
  static_assert(!std::is_assignable_v<ConstRankingIndexResult&, Picker>,
                "FAIL: The result of operator[] on a const Ranking should not be assignable.");

  static_assert(!std::is_convertible_v<std::tuple<Taste, Size, Quality>, Fruit>,
                  "FAIL: Fruit should NOT be implicitly convertible from a tuple.");

  // --- Test 2 (NEW): Explicit construction from components ---

  // We assert that Fruit is NOT copy-list-initializable from its components.
  static_assert(!is_copy_list_initializable<Taste, Size, Quality>,
                "FAIL: Fruit should NOT be copy-list-initializable. Mark the 3-arg constructor as explicit.");

  // --- We can also test that it IS directly constructible ---
  // This is the intended way: `Fruit f{t, s, q};`
  static_assert(std::is_constructible_v<Fruit, Taste, Size, Quality>,
                "FAIL: Fruit should be directly constructible from its components.");


}


} // namespace MaliciousTests


int main() {
  
// ======================== TESTS1 ========================
  test_fruit_basics();
  test_fruit_type_safety_and_stream();
  test_picker_basics_and_transforms();
  test_picker_steal_and_give();
  test_picker_equality_and_ordering();
  test_ranking_basics();
  test_ranking_merges_and_indexing();
  test_copy_move_semantics();
  test_random_invariants();

  cout << "\nALL TESTS1 PASSED!\n\n";

// ======================== TESTS2 ========================
  
  MaliciousTests::test_fruit_class();
  MaliciousTests::test_picker_class();
  MaliciousTests::test_ranking_class();
  MaliciousTests::test_language_features();
  MaliciousTests::test_compile_time_properties_with_static_assert();
  cout << "ALL TESTS2 PASSED!\n";
  
  return 0;
}
