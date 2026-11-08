#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>

#include <array>
#include <string>

#include <wx/colour.h>
#include <wx/string.h>

#include "slic3r/GUI/Widgets/StateColor.hpp"

using Catch::Generators::from_range;

namespace {

std::string hex(const wxColour& colour) { return colour.GetAsString(wxC2S_HTML_SYNTAX).ToStdString(); }

std::string dark(const char* colour) { return hex(StateColor::darkModeColorFor(wxColour(colour))); }

// The modes are process-wide; each test sets them and restores light mode.
struct ScopedColorMode
{
    ScopedColorMode(bool dark, bool oled)
    {
        StateColor::SetDarkMode(dark);
        StateColor::SetOledMode(oled);
    }
    ~ScopedColorMode()
    {
        StateColor::SetDarkMode(false);
        StateColor::SetOledMode(false);
    }
};

// Light keys of the background surfaces Black OLED mode turns black
const std::array<const char*, 6> SURFACE_KEYS{"#FFFFFF", "#3B4446", "#FEFFFF", "#F8F8F8", "#F1F1F1", "#F4F4F4"};

} // namespace

TEST_CASE("Black OLED mode darkens background surfaces below the dark grey", "[StateColor]")
{
    const char* key = GENERATE(from_range(SURFACE_KEYS));
    std::string grey;
    {
        ScopedColorMode mode(true, false);
        grey = dark(key);
    }
    ScopedColorMode mode(true, true);
    const wxColour oled = StateColor::darkModeColorFor(wxColour(key));
    CHECK(hex(oled) != grey);
    CHECK(StateColor::GetLightness(oled) < StateColor::GetLightness(wxColour(grey)));
}

TEST_CASE("Black OLED backgrounds map back to their light colour", "[StateColor]")
{
    const char* key = GENERATE(from_range(SURFACE_KEYS));
    ScopedColorMode mode(true, true);
    const wxColour oled = StateColor::darkModeColorFor(wxColour(key));
    CHECK(hex(StateColor::lightModeColorFor(oled)) == hex(wxColour(key)));
}

TEST_CASE("A second dark pass keeps Black OLED backgrounds", "[StateColor]")
{
    const char* key = GENERATE(from_range(SURFACE_KEYS));
    ScopedColorMode mode(true, true);
    const wxColour oled = StateColor::darkModeColorFor(wxColour(key));
    CHECK(hex(StateColor::darkModeColorFor(oled)) == hex(oled));
}

TEST_CASE("Switching Black OLED mode re-targets colours already mapped to dark", "[StateColor]")
{
    const char* key = GENERATE(from_range(SURFACE_KEYS));
    ScopedColorMode mode(true, false);
    const wxColour grey = StateColor::darkModeColorFor(wxColour(key));

    StateColor::SetOledMode(true);
    const wxColour oled = StateColor::darkModeColorFor(grey);
    CHECK(hex(oled) == dark(key));

    StateColor::SetOledMode(false);
    CHECK(hex(StateColor::darkModeColorFor(oled)) == hex(grey));
}

TEST_CASE("Black OLED mode keeps text and accent colours", "[StateColor]")
{
    const char* colour = GENERATE("#262E30", "#000000", "#009688", "#DFDFDF", "#DBDBDB");
    std::string grey_mode;
    {
        ScopedColorMode mode(true, false);
        grey_mode = dark(colour);
    }
    ScopedColorMode mode(true, true);
    CHECK(dark(colour) == grey_mode);
}

TEST_CASE("Black OLED mode has no effect in light mode", "[StateColor]")
{
    const char* key = GENERATE(from_range(SURFACE_KEYS));
    ScopedColorMode mode(false, true);
    CHECK(dark(key) == hex(wxColour(key)));
}
