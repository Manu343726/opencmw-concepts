#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

// clang-format off
#include <tinyrefl/api.hpp>
#include <tinyrefl/matchers.hpp>
#include "tinyrefl_test.hpp"
#include "tinyrefl_test.hpp.tinyrefl"
// clang-format on

template<typename T>
void serialize(std::ostream &os, const T &value) {
    os << "\n{\n";
    bool add_comma = false;
    tinyrefl::meta::foreach (tinyrefl::metadata<T>().children(), [&](auto child_) {
        constexpr decltype(child_) child;

        if constexpr (child.kind() == tinyrefl::entities::entity_kind::MEMBER_VARIABLE) {
            if (add_comma) {
                os << ",\n";
            }
            os << "\"" << child.name() << "\":";
            os << "\"" << child.get(value) << "\"";
            add_comma = true;
        }
    });
    os << "\n}\n";
}

// Visitors allow filtering by kind like the constexpr-if above, but work with C++14 too
template<typename T>
void serializeUsingVisitor(std::ostream &os, const T &value) {
    os << "\n{\n";
    bool add_comma = false;
    tinyrefl::visit_children(tinyrefl::metadata<T>(), tinyrefl::member_variable_visitor([&](auto variable_) {
        constexpr decltype(variable_) variable;

        if (add_comma) {
            os << ",\n";
        }
        os << "\"" << variable.name() << "\":";
        os << "\"" << variable.get(value) << "\"";
        add_comma = true;
    }));
    os << "\n}\n";
}

template<typename T>
void serializeUsingClassVisitor(std::ostream &os, const T &value) {
    os << "\n{\n";
    bool add_comma = false;
    // The difference is that this version takes into account inherited members
    tinyrefl::visit_class<T>(tinyrefl::member_variable_visitor([&](auto variable_) {
        constexpr decltype(variable_) variable;

        if (add_comma) {
            os << ",\n";
        }
        os << "\"" << variable.name() << "\":";
        os << "\"" << variable.get(value) << "\"";
        add_comma = true;
    }));
    os << "\n}\n";
}

template<typename T>
void serializeUsingSpecializedMemberVariableVisitor(std::ostream &os, const T &value) {
    os << "\n{\n";
    bool add_comma = false;
    // tinyrefl::string is like std::string_view, a constexpr string view type. But works with C++14
    tinyrefl::visit_member_variables(value, [&](const tinyrefl::string name, const auto &variable) {
        if (add_comma) {
            os << ",\n";
        }
        os << "\"" << name << "\":";
        os << "\"" << variable << "\"";
        add_comma = true;
    });
    os << "\n}\n";
}

template<typename T>
void serializeCheckingAttributes(std::ostream &os, const T &value) {
    os << "\n{\n";
    bool add_comma = false;
    // The difference is that this version takes into account inherited members
    tinyrefl::visit_class<T>(tinyrefl::member_variable_visitor([&](auto variable_) {
        constexpr decltype(variable_) variable;

        if constexpr (variable.has_attribute("test::serializable")) {
            constexpr const auto &serializable = std::get<const test::serializable &>(variable.attribute_objects());

            if constexpr (serializable) {
                if (add_comma) {
                    os << ",\n";
                }
                os << "\"" << variable.name() << "\":";
                os << "\"" << variable.get(value) << "\"";
                add_comma = true;
            }
        }
    }));
    os << "\n}\n";
}

using namespace tinyrefl::matchers;

template<typename T>
constexpr bool isOurTest() {
    return tinyrefl::matches(tinyrefl::metadata<T>(), allOf(
                                                              named("Test"),
                                                              hasParent(allOf(ofKind(tinyrefl::entities::entity_kind::NAMESPACE), named("test"))),
                                                              hasChild(named("i")),
                                                              hasChild(allOf(named("f"))),
                                                              hasChild(named("s")),
                                                              hasChild(allOf(named("function"), returns(type<void>()), hasParameter(allOf(withIndex(0), named("param")))))));
}

static_assert(isOurTest<test::Test>());

int main() {
    std::cout << "Make JSON: ";
    serialize(std::cout, test::Test{ 1, 2, "Three" });
    serializeUsingVisitor(std::cout, test::Test{ 1, 2, "Three" });
    serializeUsingClassVisitor(std::cout, test::Test{ 1, 2, "Three" });
    serializeUsingSpecializedMemberVariableVisitor(std::cout, test::Test{ 1, 2, "Three" });
    serializeCheckingAttributes(std::cout, test::Test{ 1, 2, "Three" });
    std::cout << tinyrefl::to_json(test::Test{ 1, 2, "three" }).dump() << std::endl;
    std::cout << tinyrefl::to_string(test::Test{ 1, 2, "three" }) << std::endl;

    std::cout << std::endl;
}
