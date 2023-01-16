#include"defenitions.h"

namespace chess {

	struct Bitboard {
		uint64_t data;

		constexpr Bitboard() = default;
		constexpr Bitboard(uint64_t data) :data(data) {}

		constexpr static Bitboard fromSqure(Square sq) {
			return (uint64_t)1 << sq;
		}

		constexpr bool isSet(Square sq) {
			return data & (uint64_t)1 << sq;
		}

		constexpr void set(Square sq) {
			data |= (uint64_t)1 << sq;
		}

		constexpr void unset(Square sq) {
			data &= ~((uint64_t)1 << sq);
		}

		constexpr void toggle(Square sq) {
			data ^= (uint64_t)1 << sq;
		}

		constexpr friend Bitboard operator>>(Bitboard b, uint8_t shift) {
			return b.data >> shift;
		}

		constexpr friend Bitboard operator<<(Bitboard b, uint8_t shift) {
			return b.data << shift;
		}

		constexpr bool operator==(Bitboard other) {
			return data == other.data;
		}

		constexpr bool operator!=(Bitboard other) {
			return data != other.data;
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

		constexpr void operator*=(Bitboard other) {
			data *= other.data;
		}

		constexpr Bitboard operator~() {
			return ~data;
		}

		constexpr explicit operator bool() {
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

} // namespace chess