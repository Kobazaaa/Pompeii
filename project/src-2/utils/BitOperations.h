#ifndef POMPEII_BITWISE_UTILITIES_H
#define POMPEII_BITWISE_UTILITIES_H

template<typename T> consteval static T BIT(T x) { return 1u << x; }

static void SET_BIT(auto& value, auto bit) { (value) |= BIT(bit); }
static void CLEAR_BIT(auto& value, auto bit) { (value) &= ~BIT(bit); }
static void TOGGLE_BIT(auto& value, auto bit) { (value) ^= BIT(bit); }

static bool TEST_BIT(auto value, auto bit) { return ((value)&BIT(bit)) != 0; }

static bool IS_POWER_OF_TWO(auto x) { return (x) && !((x) & ((x)-1)); }

#endif // POMPEII_BITWISE_UTILITIES_H
