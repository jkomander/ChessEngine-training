#pragma once

#include"../chess/attacks.h"

namespace chess {

	struct PGN_Move {
	};

	struct PGN_Position {
		PGN_Move move(std::string_view sv) {
			return {};
		}
	};

} // namespace chess