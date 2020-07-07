#include "big_integer.h"

using uint128_t = unsigned __int128;

static big_integer abs(big_integer const&);

big_integer::big_integer() : sign(false) {}

big_integer::big_integer(uint32_t x) : sign(false)
{
    if (x)
    {
        digits.push_back(x);
    }
}

big_integer::big_integer(int x) : sign(false)
{
    if (x)
    {
        digits.push_back(std::abs(static_cast<int64_t>(x)));
        if (x < 0)
        {
            *this = -*this;
        }
    }
}

big_integer::big_integer(std::string const& s) : big_integer()
{
    for (size_t i = (s[0] == '-' ? 1 : 0); i < s.size(); i += 8)
    {
        std::string t = s.substr(i, 8);
        uint32_t pw = 1;
        for (size_t i = 0; i < t.size(); i++)
        {
            pw *= 10;
        }
        *this *= pw;
        *this += std::stoi(t);
    }
    if (s[0] == '-')
    {
        (*this) = -(*this);
    }
}

std::string to_string(big_integer x)
{
    bool sign = x.sign;
    x = abs(x);
    std::string answer;
    while (x > 0)
    {
        big_integer r = x % big_integer(10);
        uint32_t c = (r == 0 ? 0 : r.digits.back());
        answer.push_back('0' + c);
        x /= 10;
    }
    if (sign)
    {
        answer.push_back('-');
    }
    std::reverse(answer.begin(), answer.end());

    return (answer.empty() ? "0" : answer);
}

big_integer big_integer::operator~() const
{
    big_integer x(*this);
    for (unsigned int &d : x.digits)
    {
        d ^= UINT32_MAX;
    }
    x.sign ^= static_cast<uint8_t>(1);
    x.erase_leading_zeros();
    return x;
}

big_integer big_integer::operator-() const
{
    big_integer x(*this);
    x = (~x) + 1;
    return x;
}

big_integer big_integer::operator+() const
{
    return *this;
}

big_integer operator+(big_integer l, big_integer const &r)
{
    return l += r;
}

big_integer operator-(big_integer const& a, big_integer const& b)
{
    return a + (-b);
}

big_integer operator%(big_integer const& a, big_integer const& b)
{
    return a - a / b * b;
}

big_integer operator*(big_integer const& l, big_integer const& r)
{
    big_integer a = abs(l);
    big_integer b = abs(r);
    big_integer ans;
    ans.digits.resize(a.digits.size() + b.digits.size());
    for (size_t i = 0; i < a.digits.size(); i++)
    {
        uint32_t c = 0;
        for (size_t j = 0; j < b.digits.size(); j++)
        {
            uint64_t tmp = static_cast<uint64_t>(a.digits[i]) * b.digits[j] + ans.digits[i + j] + c;
            ans.digits[i + j] = static_cast<uint32_t>(tmp & UINT32_MAX);
            c = static_cast<uint32_t>(tmp >> 32);
        }
        ans.digits[i + b.digits.size()] = c;
    }
    ans.erase_leading_zeros();
    if (l.sign != r.sign)
    {
        ans = -ans;
    }
    return ans;
}

big_integer operator/(big_integer const& l, big_integer const& r)
{
    big_integer dividend = abs(l);
    big_integer divider = abs(r);
    big_integer ans;
    big_integer ml;
    if (dividend < divider)
    {
        return 0;
    }
    if (divider.digits.size() == 1)
    {
        ans = div_short(dividend, divider.digits.back());
    }
    else
    {
        dividend.digits.push_back(0);
        size_t m = divider.digits.size() + 1;
        size_t n = dividend.digits.size();
        ans.digits.resize(n - m + 1);
        uint32_t quot = 0;
        for (size_t i = 0; i <= n - m; i++)
        {
            quot = trial(dividend, divider);
            ml = divider * quot;
            if (!smaller(dividend, abs(ml), m))
            {
                quot--;
                ml -= divider;
            }
            ans.digits[n - m - i] = quot;
            difference(dividend, abs(ml), m);
            if (!dividend.digits.back())
            {
                dividend.digits.pop_back();
            }
        }
    }
    ans.erase_leading_zeros();
    if (l.sign != r.sign)
    {
        ans = -ans;
    }
    return ans;
}

void difference(big_integer &a, big_integer const &b, size_t ind)
{
    size_t start = a.digits.size() - ind;
    bool c = false;
    for (size_t i = 0; i < ind; i++)
    {
        uint32_t x = a.digits[start + i];
        uint32_t y = (i < b.digits.size() ? b.digits[i] : 0);
        uint64_t res = static_cast<uint64_t>(x) - y - c;
        c = (y + c > x);
        a.digits[start + i] = (uint32_t)res;
    }
}

bool smaller(big_integer const &a, big_integer const &b, size_t ind)
{
    for (size_t i = 1; i <= a.digits.size(); i++)
    {
        if (a.digits[a.digits.size() - i] != (ind - i < b.digits.size() ? b.digits[ind - i] : 0))
        {
            return a.digits[a.digits.size() - i] > (ind - i < b.digits.size() ? b.digits[ind - i] : 0);
        }
    }
    return true;
}

uint32_t trial(big_integer &a, big_integer &b)
{
    uint128_t x = (static_cast<uint128_t>(a.digits[a.digits.size() - 1]) << 64) |
                  (static_cast<uint128_t>(a.digits[a.digits.size() - 2]) << 32) |
                  (static_cast<uint128_t>(a.digits[a.digits.size() - 3]));
    uint128_t y = ((static_cast<uint128_t>(b.digits[b.digits.size() - 1]) << 32) | static_cast<uint128_t>(b.digits[b.digits.size() - 2]));
    return static_cast<uint32_t>(std::min(static_cast<uint128_t>(UINT32_MAX), x / y));
}

big_integer div_short(big_integer const & dividend, uint32_t divider)
{
    big_integer res;
    uint64_t cur = 0;
    uint64_t reminder = 0;
    for (size_t i = dividend.digits.size(); i > 0; i--)
    {
        cur = reminder << 32;
        cur |= dividend.digits[i - 1];
        res.digits.push_back(static_cast<uint32_t>(cur / divider));
        reminder = cur % divider;
    }
    std::reverse(res.digits.begin(), res.digits.end());
    res.erase_leading_zeros();
    return res;
}

big_integer operator&(big_integer const& a, big_integer const& b)
{
    return bit_op(a, b, [](int32_t a, int32_t b) {return a & b;});
}

big_integer operator|(big_integer const& a, big_integer const& b)
{
    return bit_op(a, b, [](int32_t a, int32_t b) {return a | b;});
}

big_integer operator^(big_integer const& a, big_integer const& b)
{
    return bit_op(a, b, [](int32_t a, int32_t b) {return a ^ b;});
}

big_integer operator>>(big_integer const& _a, int b)
{
    big_integer a = _a;
    uint32_t cnt_digits = b / 32;
    uint32_t reminder = b % 32;
    for (size_t i = cnt_digits; i < a.digits.size(); i++)
    {
        a.digits[i - cnt_digits] = a.digits[i];
    }
    a.digits.resize(a.digits.size() - cnt_digits);
    a.erase_leading_zeros();
    big_integer _b = (static_cast<uint32_t>(1) << reminder);
    a /= _b;
    if (a < 0 && a * _b != _a)
    {
        a--;
    }
    return a;
}

big_integer operator<<(big_integer a, int b)
{
    uint32_t cnt_digits = b / 32;
    uint32_t reminder = b % 32;
    a.digits.resize(a.digits.size() + cnt_digits);
    if (cnt_digits != 0)
    {
        for (size_t i = a.digits.size(); i > cnt_digits; i--) {
            a.digits[i - 1] = a.digits[i - 1 - cnt_digits];
        }
    }
    for (size_t i = 0; i < cnt_digits; i++)
    {
        a.digits[i] = 0;
    }
    a *= (static_cast<uint32_t>(1) << reminder);
    return a;
}

big_integer& big_integer::operator++()
{
    return *this += 1;
}

big_integer const big_integer::operator++(int)
{
    big_integer x(*this);
    *this += 1;
    return x;
}

big_integer& big_integer::operator--()
{
    return *this -= 1;
}

big_integer const big_integer::operator--(int)
{
    big_integer x(*this);
    *this -= 1;
    return x;
}

big_integer& big_integer::operator+=(big_integer x)
{
    size_t MAX_LEN = std::max(digits.size(), x.digits.size());
    expand(MAX_LEN);
    x.expand(MAX_LEN);
    bool c = false;
    for (size_t i = 0; i < MAX_LEN; i++)
    {
        digits[i] += x.digits[i] + c;
        c = (digits[i] < x.digits[i] + static_cast<uint32_t>(c)) || (x.digits[i] == UINT32_MAX && c);
    }
    if (sign && x.sign)
    {
        if (!c)
        {
            digits.push_back(UINT32_MAX ^ 1);
        }
    }
    else if (c)
    {
        if (sign || x.sign)
        {
            sign = false;
        }
        else
        {
            digits.push_back(1);
        }
    }
    else
    {
        sign |= x.sign;
    }
    erase_leading_zeros();
    return *this;
}

big_integer& big_integer::operator-=(const big_integer & r)
{
    return *this += -r;
}

big_integer& big_integer::operator*=(const big_integer & r)
{
    return *this = *this * r;
}

big_integer & big_integer::operator/=(const big_integer & r)
{
    return *this = *this / r;
}

big_integer & big_integer::operator%=(const big_integer & r)
{
    return *this = *this % r;
}

big_integer & big_integer::operator&=(const big_integer & r)
{
    return *this = *this & r;
}

big_integer & big_integer::operator|=(const big_integer & r)
{
    return *this = *this | r;
}

big_integer & big_integer::operator^=(const big_integer & r)
{
    return *this = *this ^ r;
}

big_integer & big_integer::operator>>=(int x)
{
    return *this = *this >> x;
}

big_integer & big_integer::operator<<=(int x)
{
    return *this = *this << x;
}

bool operator==(big_integer const& a, big_integer const& b)
{
    if (a.sign != b.sign)
    {
        return false;
    }
    else
    {
        if (a.digits.size() != b.digits.size())
        {
            return false;
        }
        else
        {
            for (size_t i = a.digits.size(); i >= 1; i--)
            {
                if (a.digits[i - 1] != b.digits[i - 1])
                {
                    return false;
                }
            }
            return true;
        }
    }
}

bool operator<(big_integer const& a, big_integer const& b)
{
    if (a.sign != b.sign)
    {
        return a.sign;
    }
    else
    {
        if (a.digits.size() != b.digits.size())
        {
            return (a.sign && a.digits.size() > b.digits.size()) || (!a.sign &&  a.digits.size() < b.digits.size());
        }
        else
        {
            for (size_t i = a.digits.size(); i >= 1; i--)
            {
                if (a.digits[i - 1] != b.digits[i - 1])
                {
                    return a.digits[i - 1] < b.digits[i - 1];
                }
            }
            return false;
        }
    }
}

bool operator>(big_integer const& a, big_integer const& b)
{
    return b < a;
}

bool operator<=(big_integer const& a, big_integer const& b)
{
    return !(a > b);
}

bool operator>=(big_integer const& a, big_integer const& b)
{
    return !(a < b);
}

bool operator!=(big_integer const& a, big_integer const& b)
{
    return !(a == b);
}

void big_integer::expand(size_t length)
{
    while (digits.size() < length)
    {
        digits.push_back(sign ? UINT32_MAX : 0);
    }
}

void big_integer::erase_leading_zeros()
{
    while (!digits.empty() && digits.back() == (sign ? UINT32_MAX : 0))
    {
        digits.pop_back();
    }
}

big_integer bit_op(big_integer a, big_integer b, const std::function<uint32_t(uint32_t, uint32_t)>& f)
{
    size_t MAX_LEN = std::max(a.digits.size(), b.digits.size());
    a.expand(MAX_LEN);
    b.expand(MAX_LEN);
    big_integer res;
    for (size_t i = 0; i < MAX_LEN; i++)
    {
        res.digits.push_back(f(a.digits[i], b.digits[i]));
    }
    res.sign = f(a.sign, b.sign);
    res.erase_leading_zeros();
    return res;
}

static big_integer abs(big_integer const& x)
{
    return x < 0 ? -x : x;
}
