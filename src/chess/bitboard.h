#ifdef _MSC_VER
#include<intrin.h>
#endif

#include"defenitions.h"

namespace chess {

	struct Bitboard {
		uint64_t data;

		Bitboard() = default;
		constexpr Bitboard(uint64_t data) :data(data) {}

		constexpr static Bitboard fromSquare(Square sq) {
			return (uint64_t)1 << sq;
		}

		constexpr bool isSet(Square sq) {
			return data & (uint64_t)1 << sq;
		}

		constexpr void set(Square sq) {
			data |= (uint64_t)1 << sq;
		}

		constexpr void clear(Square sq) {
			data &= ~((uint64_t)1 << sq);
		}

		constexpr void toggle(Square sq) {
			data ^= (uint64_t)1 << sq;
		}

		template<Direction d>constexpr Bitboard shift();

#ifdef _MSC_VER
		int popcount() { return (int)_mm_popcnt_u64(data); }

		Square LSB() {
			unsigned long idx;
			_BitScanForward64(&idx, data);
			return Square(idx);
		}

		Square MSB() {
			unsigned long idx;
			_BitScanReverse64(&idx, data);
			return Square(idx);
		}

		void mirror() {
			data = _byteswap_uint64(data);
		}

		Bitboard mirrored() {
			return _byteswap_uint64(data);
		}

#else
		int popcount() {
			return __builtin_popcountll(data);
		}

		Square LSB() {
			return Square(__builtin_ctzll(data));
		}

		Square MSB() {
			return Square(__builtin_clzll(data) ^ 63);
		}

		void mirror() {
			constexpr uint64_t k1 = 0x00ff00ff00ff00ff;
			constexpr uint64_t k2 = 0x0000ffff0000ffff;
			data = data >> 8 & k1 | (data & k1) << 8;
			data = data >> 16 & k2 | (data & k2) << 16;
			data = data >> 32 | data << 32;
		}

		Bitboard mirrored() {
			Bitboard copy(data);
			copy.mirror();
			return copy;
#endif

		Square popLSB() {
			Square s = LSB();
			data &= data - 1;
			return s;
		}

		Square popMSB() {
			Square s = MSB();
			data &= data - 1;
			return s;
		}

		constexpr friend Bitboard operator>>(Bitboard other, uint8_t shift) {
			return other.data >> shift;
		}

		constexpr friend Bitboard operator<<(Bitboard other, uint8_t shift) {
			return other.data << shift;
		}

		constexpr bool operator==(Bitboard other) {
			return data == other.data;
		}

		constexpr friend Bitboard operator|(Bitboard a, Bitboard b) {
			return a.data | b.data;
		}

		constexpr friend Bitboard operator&(Bitboard a, Bitboard b) {
			return a.data & b.data;
		}

		constexpr friend Bitboard operator^(Bitboard a, Bitboard b) {
			return a.data ^ b.data;
		}

		constexpr friend Bitboard operator-(Bitboard a, Bitboard b) {
			return a.data & ~b.data;
		}

		constexpr friend Bitboard operator*(Bitboard a, Bitboard b) {
			return a.data * b.data;
		}

		constexpr void operator|=(Bitboard other) {
			data |= other.data;
		}

		constexpr void operator&=(Bitboard other) {
			data &= other.data;
		}

		constexpr void operator^=(Bitboard other) {
			data ^= other.data;
		}

		constexpr void operator-=(Bitboard other) {
			data &= ~other.data;
		}

		constexpr void operator*=(Bitboard other) {
			data *= other.data;
		}

		constexpr Bitboard operator~() const {
			return ~data;
		}

		constexpr explicit operator bool() const {
			return data;
		}
	};

	constexpr Bitboard FILE_A_BB(0x0101010101010101);
	constexpr Bitboard FILE_B_BB = FILE_A_BB << 1;
	constexpr Bitboard FILE_C_BB = FILE_A_BB << 2;
	constexpr Bitboard FILE_D_BB = FILE_A_BB << 3;
	constexpr Bitboard FILE_E_BB = FILE_A_BB << 4;
	constexpr Bitboard FILE_F_BB = FILE_A_BB << 5;
	constexpr Bitboard FILE_G_BB = FILE_A_BB << 6;
	constexpr Bitboard FILE_H_BB = FILE_A_BB << 7;

	constexpr Bitboard RANK_1_BB(0xff);
	constexpr Bitboard RANK_2_BB = RANK_1_BB << 8;
	constexpr Bitboard RANK_3_BB = RANK_1_BB << 16;
	constexpr Bitboard RANK_4_BB = RANK_1_BB << 24;
	constexpr Bitboard RANK_5_BB = RANK_1_BB << 32;
	constexpr Bitboard RANK_6_BB = RANK_1_BB << 40;
	constexpr Bitboard RANK_7_BB = RANK_1_BB << 48;
	constexpr Bitboard RANK_8_BB = RANK_1_BB << 56;

	constexpr Bitboard DIAG_C2_H7(0x0080402010080400);

	template<Direction d>
	constexpr Bitboard Bitboard::shift() {
		Bitboard b(*this);
		if (d == NORTH) return b << NORTH;
		else if (d == SOUTH) return b >> NORTH;
		else if (d == 2 * NORTH) return b << 2 * NORTH;
		else if (d == 2 * SOUTH) return b >> 2 * NORTH;
		else if (d == NORTHEAST) return (b << NORTHEAST) - FILE_A_BB;
		else if (d == NORTHWEST) return (b << NORTHWEST) - FILE_H_BB;
		else if (d == SOUTHEAST) return (b >> NORTHWEST) - FILE_A_BB;
		else if (d == SOUTHWEST) return (b >> NORTHEAST) - FILE_H_BB;
		else return {};
	}

} // namespace chess