#include <string>

namespace test {

struct serializable {
    constexpr explicit serializable(const bool value = true)
        : _serializable{ value } {
    }

    constexpr explicit operator bool() const {
        return _serializable;
    }

private:
    bool _serializable;
};

struct [[test::serializable]] Test {
    [[test::serializable]] float f = 0.0;
    [[test::serializable(true)]] int i = 0;
    [[test::serializable(false)]] std::string s = "";

    void function(const char *param) const;
};
} // namespace test
