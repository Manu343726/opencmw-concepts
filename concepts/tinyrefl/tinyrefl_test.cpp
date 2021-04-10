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
    // tinyrefl::metadata<T>() returns the constexpr object containing the reflection metadata of T
    // .children() returns a std::tuple<> with the metadata objects of the things declared inside the
    // T declaration "scope": members, base classes, etc.
    // tinyrefl::meta::foreach() is just an utility function to loop over a std::tuple
    tinyrefl::meta::foreach (tinyrefl::metadata<T>().children(), [&](auto child_) {
        // This weird thing is needed to make the metadata object constexpr.
        constexpr decltype(child_) child;

        // Metadata objects expose properties as member functions. kind() returns the kind of entity
        // the object reflects (A file, a namespace, a function, etc).
        if constexpr (child.kind() == tinyrefl::entities::entity_kind::MEMBER_VARIABLE) {
            if (add_comma) {
                os << ",\n";
            }

            // All entities have a name, a full (qualified) name, a display name, and a full display name.
            // The display name is the same as the name except for invokable entities (functions and constructors)
            // where it also contains the signature of the entity.
            os << "\"" << child.name() << "\":";

            // Since we selected member variables only, we know the metadata object is an
            // instance of the class representing member variables (tinyrefl::entities::member_variable).
            // This class provides a get() function to return a reference to the variable given an object of the
            // member class type.
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

// Now let's get a little deeper down the reflection hole: Let's serialize only
// the members that were marked as serializable using our custom test::serializable user-defined attribute:
template<typename T>
void serializeCheckingAttributes(std::ostream &os, const T &value) {
    os << "\n{\n";
    bool add_comma = false;
    tinyrefl::visit_class<T>(tinyrefl::member_variable_visitor([&](auto variable_) {
        constexpr decltype(variable_) variable;

        // before accessing the attribute, make sure the variable was tagged with it
        if constexpr (variable.has_attribute("test::serializable")) {
            // .attribute_objects() returns a std::tuple<> containing references to the attribute objects applied
            // to the entity, in this case the test::serializable applied to the fields of the struct.
            constexpr const auto &serializable = std::get<const test::serializable &>(variable.attribute_objects());

            // this if() calls the operator bool() implemented in test::serializable. Attributes are full fledged constexpr objects
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
