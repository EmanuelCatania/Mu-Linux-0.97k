// Sin PCH: se usa también desde los tests, fuera del ejecutable.
#include "Text/TextCore.h"
#define UTF8PROC_STATIC
#include "../../lib/utf8proc/utf8proc.h"
extern "C" {
#include "../../lib/libunibreak/linebreak.h"
}
#include <cstdlib>
#include <cstring>

namespace TextEngine {
namespace {
// 0x80..0x9F de Windows-1252. Los cinco huecos quedan como control C1.
const char16_t kWindows1252High[32] = {
    0x20AC,0x0081,0x201A,0x0192,0x201E,0x2026,0x2020,0x2021,
    0x02C6,0x2030,0x0160,0x2039,0x0152,0x008D,0x017D,0x008F,
    0x0090,0x2018,0x2019,0x201C,0x201D,0x2022,0x2013,0x2014,
    0x02DC,0x2122,0x0161,0x203A,0x0153,0x009D,0x017E,0x0178
};

bool EncodeWindows1252(char32_t codepoint, unsigned char& byte)
{
    if (codepoint < 0x80 || (codepoint >= 0xA0 && codepoint <= 0xFF)) {
        byte = static_cast<unsigned char>(codepoint);
        return true;
    }
    for (int i = 0; i < 32; ++i)
        if (kWindows1252High[i] == codepoint) { byte = static_cast<unsigned char>(0x80 + i); return true; }
    return false;
}

// Recorre los clusters: `visit(begin, end)` en bytes; corta si devuelve false.
template<class Visit>
void ForEachCluster(std::string_view text, Visit visit)
{
    std::size_t begin = 0, offset = 0;
    utf8proc_int32_t state = 0;
    char32_t previous = 0;
    bool first = true;
    while (offset < text.size()) {
        char32_t codepoint;
        const std::size_t length = Decode(text, offset, codepoint);
        if (!first && utf8proc_grapheme_break_stateful(
                static_cast<utf8proc_int32_t>(previous), static_cast<utf8proc_int32_t>(codepoint), &state)) {
            if (!visit(begin, offset)) return;
            begin = offset;
        }
        first = false;
        previous = codepoint;
        offset += length;
    }
    if (!first) visit(begin, offset);
}

std::string Map(std::string_view text, int options)
{
    const std::string valid = Sanitize(text);
    if (valid.empty()) return {};
    utf8proc_uint8_t* mapped = nullptr;
    const utf8proc_ssize_t length = utf8proc_map(
        reinterpret_cast<const utf8proc_uint8_t*>(valid.data()),
        static_cast<utf8proc_ssize_t>(valid.size()), &mapped,
        static_cast<utf8proc_option_t>(options));
    if (length < 0 || !mapped) return valid;
    std::string result(reinterpret_cast<const char*>(mapped), static_cast<std::size_t>(length));
    std::free(mapped);
    return result;
}
} // namespace

char32_t DecodeWindows1252(unsigned char byte)
{
    return byte >= 0x80 && byte < 0xA0 ? kWindows1252High[byte - 0x80] : byte;
}

std::string FromWindows1252(std::string_view text)
{
    std::string out;
    out.reserve(text.size());
    for (const char c : text) Append(out, DecodeWindows1252(static_cast<unsigned char>(c)));
    return out;
}

std::string ToWindows1252(std::string_view text, char replacement)
{
    const std::string normalized = NormalizeNFC(text);
    std::string out;
    out.reserve(normalized.size());
    ForEachCluster(normalized, [&](std::size_t begin, std::size_t end) {
        char32_t codepoint;
        const std::size_t length = Decode(normalized, begin, codepoint);
        unsigned char byte;
        if (begin + length == end && EncodeWindows1252(codepoint, byte))
            out.push_back(static_cast<char>(byte));
        else
            out.push_back(replacement);
        return true;
    });
    return out;
}

std::size_t Decode(std::string_view text, std::size_t offset, char32_t& codepoint)
{
    if (offset >= text.size()) { codepoint = 0; return 0; }
    const auto* s = reinterpret_cast<const unsigned char*>(text.data()) + offset;
    const std::size_t available = text.size() - offset;
    const unsigned char lead = s[0];
    std::size_t length;
    char32_t value, minimum;
    if (lead < 0x80) { codepoint = lead; return 1; }
    else if ((lead & 0xE0) == 0xC0) { length = 2; value = lead & 0x1F; minimum = 0x80; }
    else if ((lead & 0xF0) == 0xE0) { length = 3; value = lead & 0x0F; minimum = 0x800; }
    else if ((lead & 0xF8) == 0xF0) { length = 4; value = lead & 0x07; minimum = 0x10000; }
    else { codepoint = ReplacementCharacter; return 1; }
    if (available < length) { codepoint = ReplacementCharacter; return 1; }
    for (std::size_t i = 1; i < length; ++i) {
        if ((s[i] & 0xC0) != 0x80) { codepoint = ReplacementCharacter; return 1; }
        value = (value << 6) | (s[i] & 0x3F);
    }
    if (value < minimum || value > 0x10FFFF || (value >= 0xD800 && value <= 0xDFFF)) {
        codepoint = ReplacementCharacter;
        return 1;
    }
    codepoint = value;
    return length;
}

void Append(std::string& out, char32_t codepoint)
{
    if (codepoint > 0x10FFFF || (codepoint >= 0xD800 && codepoint <= 0xDFFF))
        codepoint = ReplacementCharacter;
    if (codepoint < 0x80) {
        out.push_back(static_cast<char>(codepoint));
    } else if (codepoint < 0x800) {
        out.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
        out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else if (codepoint < 0x10000) {
        out.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
        out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
        out.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
}

bool IsValid(std::string_view text)
{
    for (std::size_t offset = 0; offset < text.size();) {
        char32_t codepoint;
        const std::size_t length = Decode(text, offset, codepoint);
        // U+FFFD escrito tal cual ocupa 3 bytes; el de error, 1.
        if (codepoint == ReplacementCharacter && length == 1) return false;
        offset += length;
    }
    return true;
}

std::string Sanitize(std::string_view text)
{
    if (IsValid(text)) return std::string(text);
    std::string out;
    out.reserve(text.size() + 8);
    for (std::size_t offset = 0; offset < text.size();) {
        char32_t codepoint;
        offset += Decode(text, offset, codepoint);
        Append(out, codepoint);
    }
    return out;
}

std::size_t CodepointCount(std::string_view text)
{
    std::size_t count = 0;
    for (std::size_t offset = 0; offset < text.size(); ++count) {
        char32_t codepoint;
        offset += Decode(text, offset, codepoint);
    }
    return count;
}

std::size_t ClusterCount(std::string_view text)
{
    std::size_t count = 0;
    ForEachCluster(text, [&](std::size_t, std::size_t) { ++count; return true; });
    return count;
}

std::size_t NextCluster(std::string_view text, std::size_t offset)
{
    if (offset >= text.size()) return text.size();
    std::size_t next = text.size();
    ForEachCluster(text, [&](std::size_t, std::size_t end) {
        if (end > offset) { next = end; return false; }
        return true;
    });
    return next;
}

std::size_t PreviousCluster(std::string_view text, std::size_t offset)
{
    if (offset > text.size()) offset = text.size();
    std::size_t previous = 0;
    ForEachCluster(text, [&](std::size_t begin, std::size_t) {
        if (begin >= offset) return false;
        previous = begin;
        return true;
    });
    return previous;
}

std::size_t FitBytes(std::string_view text, std::size_t maxBytes)
{
    if (text.size() <= maxBytes) return text.size();
    std::size_t fit = 0;
    ForEachCluster(text, [&](std::size_t, std::size_t end) {
        if (end > maxBytes) return false;
        fit = end;
        return true;
    });
    return fit;
}

std::size_t FitClusters(std::string_view text, std::size_t maxClusters)
{
    std::size_t fit = 0, count = 0;
    ForEachCluster(text, [&](std::size_t, std::size_t end) {
        if (count == maxClusters) return false;
        ++count;
        fit = end;
        return true;
    });
    return fit;
}

std::size_t CopyTruncated(char* out, std::size_t capacity, std::string_view text)
{
    if (!out || !capacity) return 0;
    const std::size_t length = FitBytes(text, capacity - 1);
    if (length) std::memcpy(out, text.data(), length);
    out[length] = '\0';
    return length;
}

std::size_t ConvertWindows1252InPlace(char* field, std::size_t capacity)
{
    if (!field || !capacity) return 0;
    std::size_t length = 0;
    while (length < capacity && field[length]) ++length;
    bool ascii = true;
    for (std::size_t i = 0; i < length; ++i)
        if (static_cast<unsigned char>(field[i]) >= 0x80) { ascii = false; break; }
    if (ascii) {
        if (length == capacity) { field[capacity - 1] = 0; return capacity - 1; }
        return length;
    }
    const std::string converted = FromWindows1252(std::string_view(field, length));
    return CopyTruncated(field, capacity, converted);
}

std::size_t NormalizeTextFileInPlace(char* field, std::size_t capacity)
{
    if (!field || !capacity) return 0;
    std::size_t length = 0;
    while (length < capacity && field[length]) ++length;
    if (length < capacity && IsValid(std::string_view(field, length))) return length;
    return ConvertWindows1252InPlace(field, capacity);
}

std::string NormalizeNFC(std::string_view text)
{
    return Map(text, UTF8PROC_STABLE | UTF8PROC_COMPOSE);
}

std::string CaseFoldKey(std::string_view text)
{
    return Map(text, UTF8PROC_STABLE | UTF8PROC_COMPOSE | UTF8PROC_CASEFOLD);
}

std::vector<unsigned char> LineBreaks(std::string_view text)
{
    std::vector<unsigned char> result(text.size() + 1, 0);
    if (text.empty()) return result;
    static const bool initialized = [] { init_linebreak(); return true; }();
    (void)initialized;
    // libunibreak marca el estado DESPUÉS de cada byte; acá se guarda como
    // "antes del byte siguiente". Una secuencia inválida se corta como un byte.
    std::vector<char> after(text.size());
    set_linebreaks_utf8(reinterpret_cast<const utf8_t*>(text.data()), text.size(), nullptr, after.data());
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (after[i] == LINEBREAK_MUSTBREAK) result[i + 1] = 2;
        else if (after[i] == LINEBREAK_ALLOWBREAK) result[i + 1] = 1;
    }
    // El final del texto no es un corte: es simplemente el final.
    result[text.size()] = 0;
    return result;
}

std::size_t FitLineBreak(std::string_view text, std::size_t maxBytes)
{
    const auto breaks = LineBreaks(text);
    const std::size_t limit = maxBytes < text.size() ? maxBytes : text.size();
    // Un salto obligatorio antes del tope manda.
    for (std::size_t i = 1; i <= limit; ++i)
        if (breaks[i] == 2) return i;
    if (maxBytes >= text.size()) return text.size();
    for (std::size_t i = maxBytes; i > 0; --i)
        if (breaks[i]) return i;
    return 0;
}

namespace {
struct Emoticon { const char* code; const char* emoji; };
// Palabras sueltas: se reconocen sólo entre espacios o en los bordes, así una
// URL ("http://") o un horario no se tocan.
const Emoticon kEmoticons[] = {
    { ":)", u8"\U0001F642" }, { ":-)", u8"\U0001F642" }, { "=)", u8"\U0001F642" },
    { ":D", u8"\U0001F600" }, { ":-D", u8"\U0001F600" }, { "=D", u8"\U0001F600" },
    { "xD", u8"\U0001F606" }, { "XD", u8"\U0001F606" },
    { ";)", u8"\U0001F609" }, { ";-)", u8"\U0001F609" },
    { ":(", u8"\U0001F641" }, { ":-(", u8"\U0001F641" },
    { ":'(", u8"\U0001F622" }, { ":'-(", u8"\U0001F622" }, { "T_T", u8"\U0001F62D" },
    { ":P", u8"\U0001F61B" }, { ":p", u8"\U0001F61B" }, { ":-P", u8"\U0001F61B" }, { ":-p", u8"\U0001F61B" },
    { ";P", u8"\U0001F61C" }, { ";p", u8"\U0001F61C" },
    { ":O", u8"\U0001F62E" }, { ":o", u8"\U0001F62E" }, { ":-O", u8"\U0001F62E" },
    { ":*", u8"\U0001F618" }, { ":-*", u8"\U0001F618" },
    { ":|", u8"\U0001F610" }, { ":-|", u8"\U0001F610" }, { "-_-", u8"\U0001F611" },
    { ":/", u8"\U0001F615" }, { ":-/", u8"\U0001F615" }, { ":$", u8"\U0001F633" },
    { "B)", u8"\U0001F60E" }, { "B-)", u8"\U0001F60E" }, { "8)", u8"\U0001F60E" },
    { ">:(", u8"\U0001F620" }, { "^^", u8"\U0001F60A" }, { "^_^", u8"\U0001F60A" },
    { "<3", u8"\u2764\uFE0F" }, { "</3", u8"\U0001F494" },
};
// ":nombre:" en cualquier parte del texto.
const Emoticon kShortcodes[] = {
    { "smile", u8"\U0001F604" }, { "grin", u8"\U0001F601" }, { "joy", u8"\U0001F602" },
    { "wink", u8"\U0001F609" }, { "cool", u8"\U0001F60E" }, { "love", u8"\U0001F60D" },
    { "cry", u8"\U0001F622" }, { "sob", u8"\U0001F62D" }, { "angry", u8"\U0001F620" },
    { "think", u8"\U0001F914" }, { "sleep", u8"\U0001F634" }, { "eyes", u8"\U0001F440" },
    { "heart", u8"\u2764\uFE0F" }, { "fire", u8"\U0001F525" }, { "star", u8"\u2B50" },
    { "100", u8"\U0001F4AF" }, { "skull", u8"\U0001F480" }, { "party", u8"\U0001F389" },
    { "thumbsup", u8"\U0001F44D" }, { "thumbsdown", u8"\U0001F44E" }, { "clap", u8"\U0001F44F" },
    { "ok", u8"\U0001F44C" }, { "wave", u8"\U0001F44B" }, { "pray", u8"\U0001F64F" },
    { "muscle", u8"\U0001F4AA" }, { "sword", u8"\u2694\uFE0F" }, { "shield", u8"\U0001F6E1\uFE0F" },
    { "gem", u8"\U0001F48E" }, { "gift", u8"\U0001F381" }, { "trophy", u8"\U0001F3C6" },
    { "zen", u8"\U0001F4B0" }, { "dragon", u8"\U0001F409" }, { "beer", u8"\U0001F37A" },
};
}

std::string ExpandEmoticons(std::string_view text)
{
    std::string out;
    bool changed = false;
    std::size_t i = 0;
    while (i < text.size()) {
        if (text[i] == ' ' || text[i] == '\t') { out.push_back(text[i++]); continue; }
        std::size_t end = i;
        while (end < text.size() && text[end] != ' ' && text[end] != '\t') ++end;
        const std::string_view word = text.substr(i, end - i);
        bool whole = false;
        for (const Emoticon& e : kEmoticons)
            if (word == e.code) { out += e.emoji; whole = changed = true; break; }
        if (!whole) {
            // Códigos ":nombre:" dentro de la palabra.
            for (std::size_t k = 0; k < word.size();) {
                bool replaced = false;
                if (word[k] == ':') {
                    const std::size_t close = word.find(':', k + 1);
                    if (close != std::string_view::npos) {
                        const std::string_view name = word.substr(k + 1, close - k - 1);
                        for (const Emoticon& e : kShortcodes)
                            if (name == e.code) { out += e.emoji; k = close + 1; replaced = changed = true; break; }
                    }
                }
                if (!replaced) out.push_back(word[k++]);
            }
        }
        i = end;
    }
    return changed ? out : std::string(text);
}

bool IsNameText(std::string_view text, std::size_t maxClusters)
{
    if (text.empty() || !IsValid(text) || NormalizeNFC(text) != text) return false;
    const std::size_t clusters = ClusterCount(text);
    if (clusters < 1 || clusters > maxClusters) return false;
    bool first = true;
    for (std::size_t at = 0; at < text.size();) {
        char32_t codepoint = 0;
        at += Decode(text, at, codepoint);
        const utf8proc_category_t category = utf8proc_category((utf8proc_int32_t)codepoint);
        const bool letter = category >= UTF8PROC_CATEGORY_LU && category <= UTF8PROC_CATEGORY_LO;
        const bool mark = category >= UTF8PROC_CATEGORY_MN && category <= UTF8PROC_CATEGORY_ME;
        const bool digit = category >= UTF8PROC_CATEGORY_ND && category <= UTF8PROC_CATEGORY_NO;
        if (!(letter || digit || (mark && !first))) return false;
        first = false;
    }
    return true;
}

int CompareIgnoreCase(std::string_view left, std::string_view right)
{
    return CaseFoldKey(left).compare(CaseFoldKey(right));
}

} // namespace TextEngine
