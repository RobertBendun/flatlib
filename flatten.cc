#ifndef FLATLIB
#define FLATLIB

#include <ranges>

namespace flatlib
{
	template <typename Iter> struct flat_view;

	template <typename Range>
		requires(!std::ranges::range<std::ranges::range_value_t<Range>>)
	struct flat_view<Range> : std::ranges::view_interface<flat_view<Range>>
	{
		using underlying_iterator = std::ranges::iterator_t<Range>;

		using value_type =
				typename std::iterator_traits<underlying_iterator>::value_type;
		using reference =
				typename std::iterator_traits<underlying_iterator>::reference;
		using iterator_category =
				typename std::iterator_traits<underlying_iterator>::iterator_category;
		using difference_type =
				typename std::iterator_traits<underlying_iterator>::difference_type;

		std::ranges::iterator_t<Range> m_begin;
		std::ranges::sentinel_t<Range> m_end;

		constexpr flat_view() noexcept = default;
		constexpr flat_view(flat_view const &) noexcept = default;
		constexpr flat_view(flat_view &&) noexcept = default;
		constexpr flat_view &operator=(flat_view const &) noexcept = default;
		constexpr flat_view &operator=(flat_view &&) noexcept = default;

		flat_view(Range &range)
				: m_begin(std::begin(range)), m_end(std::end(range)) {}

		auto begin() { return m_begin; }
		auto end() { return m_end; }

		reference operator*() const { return *m_begin; }

		auto &operator++()
		{
			if (m_begin != m_end) {
				++m_begin;
			}
			return *this;
		}

		auto operator++(int)
		{
			auto copy = *this;
			this->operator++();
			return copy;
		}

		auto &operator--()
			requires std::bidirectional_iterator<underlying_iterator>
		{
			--m_begin;
			return *this;
		}

		auto operator--(int)
			requires std::bidirectional_iterator<underlying_iterator>
		{
			auto copy = *this;
			this->operator--();
			return copy;
		}

		bool operator==(flat_view const &other) const {
			return (m_begin == m_end && other.m_begin == other.m_end) ||
						 m_begin == other.m_begin;
		}

		auto operator<=>(flat_view const &other) const
			requires std::random_access_iterator<underlying_iterator>
		{
			return m_begin <=> other.m_begin;
		}

		difference_type operator-(flat_view const &other) const
			requires std::random_access_iterator<underlying_iterator>
		{
			return m_begin - other.m_begin;
		}

		auto &operator+=(difference_type n)
			requires std::random_access_iterator<underlying_iterator>
		{
			m_begin += n;
			return *this;
		}

		auto operator+(difference_type n) const
			requires std::random_access_iterator<underlying_iterator>
		{
			auto copy = *this;
			copy += n;
			return copy;
		}

		friend auto operator+(difference_type n, flat_view view)
			requires std::random_access_iterator<underlying_iterator>
		{
			return view + n;
		}

		auto &operator-=(difference_type n)
			requires std::random_access_iterator<underlying_iterator>
		{
			m_begin -= n;
			return *this;
		}

		auto operator-(difference_type n) const
			requires std::random_access_iterator<underlying_iterator>
		{
			auto copy = *this;
			copy -= n;
			return copy;
		}

		reference operator[](difference_type n) const
			requires std::random_access_iterator<underlying_iterator>
		{
			return *(m_begin + n);
		}
	};

	template <typename Range>
		requires std::ranges::range<std::ranges::range_value_t<Range>>
	struct flat_view<Range> : std::ranges::view_interface<flat_view<Range>>
	{
		using range_iterator = std::ranges::iterator_t<Range>;
		using range_sentinel = std::ranges::sentinel_t<Range>;
		using subrange = flat_view<std::ranges::range_value_t<Range>>;

		using value_type = typename subrange::value_type;
		using reference = typename subrange::reference;

		// flat_view of dynamic containers can be at most std::bidirectional_iterator
		// Maybe flat_view of compile time known sizes: like
		// std:;array<std::array<int, N>, M> will be random access?
		using iterator_category =
				std::conditional_t<std::is_same_v<typename subrange::iterator_category,
																					std::input_iterator_tag>,
													 std::input_iterator_tag, std::forward_iterator_tag>;

		using difference_type = std::ptrdiff_t;

		// using range_sentinel = std::ranges::sentinel_t<std::iter_value_t<Iter>>;

		range_sentinel m_range_end;
		std::optional<range_iterator> m_range_begin = std::nullopt;
		mutable std::optional<subrange> m_subrange = std::nullopt;

		constexpr flat_view() noexcept = default;
		constexpr flat_view(flat_view const &) noexcept = default;
		constexpr flat_view(flat_view &&) noexcept = default;
		constexpr flat_view &operator=(flat_view const &) noexcept = default;
		constexpr flat_view &operator=(flat_view &&) noexcept = default;

		flat_view(Range &range)
			: m_range_end(std::end(range))
			, m_range_begin(std::begin(range))
		{
		}

		reference operator*() const
		{
			if (!m_subrange) {
				m_subrange.emplace(**m_range_begin);
			}
			return *m_subrange->begin();
		}

		flat_view &operator++()
		{
			if (!m_range_begin || *m_range_begin == m_range_end) {
				return *this;
			}
			if (m_subrange) {
				m_subrange = std::next(*m_subrange);
			}
			if (!m_subrange || m_subrange->begin() == m_subrange->end()) {
				if (++*m_range_begin == m_range_end) {
					return *this;
				}
				m_subrange.emplace(**m_range_begin);
			}
			return *this;
		}

		flat_view operator++(int)
		{
			auto copy = *this;
			this->operator++();
			return copy;
		}

		flat_view &operator--()
			requires(std::bidirectional_iterator<range_iterator> &&
							 std::bidirectional_iterator<std::ranges::iterator_t<subrange>>)
		= delete; // TODO

		bool operator==(flat_view const &other) const noexcept
		{
			if (m_range_begin && other.m_range_begin) {
				if (*m_range_begin != *other.m_range_begin) {
					return false;
				}
				if (m_subrange && other.m_subrange) {
					return m_subrange->begin() == other.m_subrange->begin();
				}
				if (!m_subrange && !other.m_subrange) {
					return true;
				}
				if (m_subrange) {
					return m_subrange->begin() == (*m_range_begin)->begin();
				}
				return other.m_subrange->begin() == (*other.m_range_begin)->begin();
			}

			if (m_range_begin) {
				return *m_range_begin == m_range_end ||
							 *m_range_begin == other.m_range_end;
			}
			if (other.m_range_begin) {
				return other.m_range_begin == other.m_range_end;
			}

			return true;
		}

		auto begin() { return *this; }

		auto end() {
			flat_view copy = *this;
			copy.m_subrange = std::nullopt;
			copy.m_range_begin = std::nullopt;
			return copy;
		}
	};

	template <std::ranges::range Range> auto flat(Range &range)
	{
		return flat_view<Range>{range};
	}
}

#endif

#ifdef FLATLIB_TEST

#include <array>
#include <forward_list>
#include <iomanip>
#include <iostream>
#include <list>
#include <numeric>
#include <source_location>
#include <sstream>
#include <vector>

static_assert(
		std::forward_iterator<flatlib::flat_view<std::forward_list<int>>>);
static_assert(std::bidirectional_iterator<flatlib::flat_view<std::list<int>>>);
static_assert(
		std::random_access_iterator<flatlib::flat_view<std::vector<int>>>);

static_assert(std::forward_iterator<
							flatlib::flat_view<std::forward_list<std::vector<int>>>>);
static_assert(
		std::forward_iterator<flatlib::flat_view<std::list<std::vector<int>>>>);

static_assert(
		std::same_as<
				flatlib::flat_view<std::vector<std::vector<int>>>::iterator_category,
				std::forward_iterator_tag>);

void test(std::string_view expected_output, auto code,
					std::source_location sl = std::source_location::current()) {
	std::stringstream ss;
	code(ss);
	if (ss.str() != expected_output) {
		std::cout << std::flush;
		std::cerr << "[ERROR] in test defined on line " << sl.line() << std::endl;
		std::cerr << "  Expected: " << std::quoted(expected_output) << std::endl;
		std::cerr << "       Got: " << std::quoted(ss.str()) << std::endl;
	}
}

int main() {
	test("123456", [](std::stringstream &out) {
		std::stringstream ss1, ss2;
		ss1 << "123";
		ss2 << "456";

		auto list_of_stream_iterators = std::forward_list<std::ranges::subrange<
				std::istream_iterator<char>, std::istream_iterator<char>>>{
				{std::istream_iterator<char>{ss1}, std::istream_iterator<char>{}},
				{std::istream_iterator<char>{ss2}, std::istream_iterator<char>{}}};

		for (auto &el : flatlib::flat(list_of_stream_iterators)) {
			out << el;
		}
	});

	test("255255255000", [](std::stringstream &out) {
		std::vector<std::array<int, 3>> colors = {{0xff, 0xff, 0xff},
																							{0, 0, 0}};
		for (auto &el : flatlib::flat(colors)) {
			out << el;
		}
	});

	test("11 22 \n33 44 55 66 \n", [](std::stringstream &out) {
		std::vector<std::vector<int>> nums = { {1,2}, {3,4,5,6} };
		for (auto &el : flatlib::flat(nums)) {
			el += el * 10;
		}
		for (auto const& vec : nums) {
			for (auto const& num : vec) {
				out << num << ' ';
			}
			out << '\n';
		}
	});
}

#endif
