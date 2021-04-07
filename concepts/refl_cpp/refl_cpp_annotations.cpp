#include <https://raw.githubusercontent.com/veselink1/refl-cpp/master/include/refl.hpp>
#include <algorithm>
#include <iostream>
#include <type_traits>
#include <assert.h>

// some type annotation concepts -- food-for-thought and discission - author: rstein


/* just some helper function to return nicer human-readable type names */
template<typename T> // N.B. extend this for custom classes using type-traits to query nicer class-type name
constexpr const char *getNicerTypeName() /* const */ noexcept {
    if (std::is_same<T, std::byte>::value) {
        return "byte";
    }
    if (std::is_same<T, char>::value) {
        return "char";
    }
    if (std::is_same<T, short>::value) {
        return "short";
    }
    if (std::is_same<T, int>::value) {
        return "int";
    }
    if (std::is_same<T, long>::value) {
        return "long";
    }
    if (std::is_same<T, float>::value) {
        return "float";
    }
    if (std::is_same<T, double>::value) {
        return "double";
    }
    if (std::is_same<T, std::string>::value) {
        return "string";
    }

    return typeid(T).name();
}

/* unit-/description-type annotation variant #1 C++17 compatible */
template<typename T>
struct Annotated0 {
    T value;
    const char *unit;
    const char *description;

    constexpr Annotated0(const T &initValue, const char *initUnit = "", const char *initDescription = "") noexcept : value(initValue), unit(initUnit), description(initDescription) { }
    constexpr operator T& () { return value; }
    constexpr const char *getUnit() const noexcept { return unit;}
    constexpr const char *getDescription() const noexcept { return description;}

    //constexpr auto operator<=>(const Annotated0&) const noexcept = default; // TODO: needs special implementation to check only value
    constexpr void operator+=(const T &a) noexcept { value += a; }
    constexpr void operator-=(const T &a) noexcept { value -= a; }
    constexpr void operator*=(const T &a) noexcept { value *= a; }
    constexpr void operator/=(const T &a) { value /= a; }

    constexpr const char *getTypeName() const noexcept { return getNicerTypeName<T>(); }
    friend constexpr std::ostream &operator<<(std::ostream &os, const Annotated0& m) { return os << m.value; }
};

template<size_t N>
struct StringLiteral {
    char value[N]{};

    constexpr StringLiteral(const char (&str)[N]) {
        std::copy_n(str, N, value);
    }
};

/* unit-/description-type annotation variant #2 C++20 compatible (to note: cool non-type template arguments
    and of course the space-ship <=> operator */
template<typename T, const StringLiteral unit = "", const StringLiteral description = "">
struct Annotated1 {
    T value;
    constexpr Annotated1(const T &initValue) noexcept : value(initValue) { }
    constexpr operator T& () { return value; }
    constexpr const char *getUnit() const noexcept { return unit.value; }
    constexpr const char *getDescription() const noexcept { return description.value; }

    constexpr auto operator<=>(const Annotated1&) const noexcept = default;
    template<typename T2, const StringLiteral ounit = "", const StringLiteral odescription = "">
    constexpr const bool operator==(const Annotated1<T2, ounit, odescription>& rhs) const noexcept {
        //TODO: optimise
        if (value != rhs.value) { // check if 'value' matches
            return false;
        }
        if (getUnit() != rhs.getUnit()) { // check if 'unit' matches
            return false; // throw an exception alternatively ??
        }
        return true;
    };
    constexpr void operator+=(const T &a) noexcept { value += a; }
    constexpr void operator-=(const T &a) noexcept { value -= a; }
    constexpr void operator*=(const T &a) noexcept { value *= a; }
    constexpr void operator/=(const T &a) { value /= a; }
    constexpr void operator*=(const Annotated1 &a) noexcept {
        value *= a.value; // N.B. actually also changes 'unit' -- implement? Nice semmantic but performance....?
    }
    constexpr const char *getTypeName() const noexcept { return getNicerTypeName<T>(); }
    friend constexpr std::ostream &operator<<(std::ostream &os, const Annotated1& m) { return os << m.value; }
};

/* unit-/description-type annotation variant #3 C++17 compatible and using refl-cpp's PoCo--attributes */
struct Annotated2 : refl::attr::usage::field {
    const char *unit;
    const char *description;
    constexpr Annotated2(const char *initUnit = "", const char *initDescription = "") noexcept : unit(initUnit), description(initDescription) { }
    constexpr const char *getUnit() const noexcept { return unit; }
    constexpr const char *getDescription() const noexcept { return description; }
};


/* some print-out/debug helper -- ignore */
int counter = 0;
template<typename T>
constexpr void printMetaInfo(const Annotated0<T> &value) {
    std::cout << "value" << ++counter << ": '" << value << "' type: '" << value.getTypeName() << "' unit: '" << value.unit << "' description: '" << value.description << "'\n";
}


template<typename T, const StringLiteral U, const StringLiteral V>
constexpr void printMetaInfo(const Annotated1<T, U, V>& value) {
    std::cout << "value" << ++counter << ": '" << value << "' type: '" << value.getTypeName() << "' unit: '" << value.getUnit() << "' description: '" << value.getDescription() << "'\n";
}

template <typename T>
void serialise(T&& value, const int hierarchyDepth = 0, std::ostream& os = std::cout) {
    if (hierarchyDepth == 0) {
        os << "\nserialise class: '" << refl::reflect(value).name << "'\n";
    }
    for_each(refl::reflect(value).members, [&](auto member) {
        // TODO: do we need this serialisable attribute or do we serialise all members except those that are impossible and or marked by Ignore<type>
        if constexpr (is_readable(member) && refl::descriptor::has_attribute<Annotated2>(member)) {
            constexpr Annotated2 meta = refl::descriptor::get_attribute<Annotated2>(member);
            for( int num_sp = 0 ; num_sp < 2*hierarchyDepth+2 ; ++num_sp ) {
                os << ' '; // there must be something smarter than this
            }
            auto fieldValue = member(value);
            os << getNicerTypeName<decltype(fieldValue)>() << " '" << get_display_name(member) << "' = '" << fieldValue << "' [" << meta.getUnit() << "] - desc: '" << meta.description << "';\n";
            serialise(member(value), hierarchyDepth + 1, os);
        }
    });
}

// ####################### user mock-up code starts here #######################

struct OtherStruct {
    const char* name = "test";
    int a;
    int b;

    friend std::ostream &operator<<(std::ostream &os, const OtherStruct& m) {
        return os << "(" << m.name << ", " << m.a << ", " << m.b << ")";
    }
};

REFL_TYPE(OtherStruct)
REFL_FIELD(name, Annotated2("no unit", "custom description for name"))
REFL_FIELD(a, Annotated2())
REFL_FIELD(b, Annotated2())
REFL_END

struct DomainObject {
    float x;
    float y;
    float z; // <- ignored through missing Annotated2 attribute (see below)

    OtherStruct innerClass;

    friend constexpr std::ostream &operator<<(std::ostream &os, const DomainObject& m) {
        return os << '(' << m.x << ", " << m.y << ", " << m.z << ", " << m.innerClass << ')';
    }
};

REFL_TYPE(DomainObject)
REFL_FIELD(x, Annotated2("m", "x coordinate"))
REFL_FIELD(y, Annotated2("m", "y coordinate"))
REFL_FIELD(z) // <- missing 'Annotated2' attribute object declares ignore for (de-)serialisation
REFL_FIELD(innerClass, Annotated2("", "an inner class"))
REFL_END

#pragma GCC diagnostic ignored "-Wunknown-attributes"
int main() {
    // style option #1
    Annotated0<int>         val1(10);
    constexpr Annotated0<double>      val2(12.0, "m", "custom description");
    Annotated0<std::string> val3("Hello", "A", "a string");
    constexpr Annotated0<double>      val4 = 10.0;

    // user-defined annotations idea:
    // similar to Java's annotation but not (and unlikely future) support by C++ standard (compile-/run-time-difference <-> static vs. RT reflections)
    // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0283r2.html
    // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0283r1.pdf
    // other issues: missing attribute type safety, require code-parser/generator, just for discussion reference
    // N.B. warning masked by line 177 (#pragma GCC ...)
    [[unit("m"), description("")]]
    double val4b = 10.0;
    [[unit("m"), description("special documentation")]]
    double val4c = 10.0;
    [[unit("m"), description("comment")]]
    double val4d = 10.0;
    [[unit("m"), description("")]]
    double val4e = 10.0;
    val1 = 11;
    val1 += 100;
    val3 += " World!";

    // style option #2
    Annotated1<float,  "K",  "an important float">          val5(10.0);
    constexpr Annotated1<short,  "°C", "an important short">          val6 = 2;
    constexpr Annotated1<short,  "°C", "yet another important short"> val7 = 3;
    constexpr Annotated1<short,  "°C", "yet another important short"> val8 = 4;
    constexpr Annotated1<short,  "K",  "yet another important short"> val9 = 4; // N.B. different unit
    constexpr Annotated1<double, "K",  "yet another important short"> val10 = 4.0; // N.B. different type
    constexpr Annotated1<double, "K",  "description doesn't matter">  val11 = 4.0; // N.B. different type

    Annotated1<double> importantValue = 10;
    //Ignore<double> unimportantValue = 20;

    // macro: addMetaInfo(Type (class/struct), importantValue, fieldNameA, fieldNameC)

    val5 += 0.1f;

    printMetaInfo(val1);
    printMetaInfo(val2);
    printMetaInfo(val3);
    printMetaInfo(val4);
    printMetaInfo(val5);
    printMetaInfo(val6);
    printMetaInfo(val7);
    printMetaInfo(val8);
    printMetaInfo(val9);
    printMetaInfo(val10);
    printMetaInfo(val11);

    assert(val7.getDescription() == val8.getDescription()); // same description text
    assert(val7 == val7);   // qual values
    assert(val7 != val8);   // different values, same unit
    assert(val7 <= val8);   // different values, same unit
    assert(val7 < val8);    // different values, same unit
    assert(val8 > val7);    // different values, same unit

    assert(val8 != val9);   // same value, different unit
    assert(val9 == val10);  // same value, same unit, different type
    assert(val10 == val11); // same value, same unit, same type, different description

    // specific example of traversing demo domain-object using refl-cpp and read macro-based annotatons
    serialise(DomainObject{ 1,2,3});

    return val1;
}