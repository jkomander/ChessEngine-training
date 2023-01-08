import numpy as np
import torch

NUM_FEATURES = 41916
PADDED_NUM_FEATURES = 41920
HALF_INPUT_SIZE = PADDED_NUM_FEATURES
HALF_ACCUMULATOR_SIZE = 256
INPUT_SIZE = 2*HALF_INPUT_SIZE
ACCUMULATOR_SIZE = 2*HALF_ACCUMULATOR_SIZE
HIDDEN_1_SIZE = 32
HIDDEN_2_SIZE = 32
OUTPUT_SIZE = 1
MAX_ACTIVE_FEATURES = 37

WHITE = 0
BLACK = 1

INPUT_SCALE = 127.
OUTPUT_DIVISOR = 26.

WEIGHT_SCALE_ACCUMULATOR = 127.
WEIGHT_SCALE_HIDDEN_1 = 128.
WEIGHT_SCALE_HIDDEN_2 = 64.
WEIGHT_SCALE_OUT = 64.

WEIGHT_MAX_HIDDEN_1    = INPUT_SCALE / WEIGHT_SCALE_HIDDEN_1
WEIGHT_MAX_HIDDEN_2    = INPUT_SCALE / WEIGHT_SCALE_HIDDEN_2
WEIGHT_MAX_OUT         = INPUT_SCALE / WEIGHT_SCALE_OUT

WEIGHT_MIN_HIDDEN_1    = -WEIGHT_MAX_HIDDEN_1
WEIGHT_MIN_HIDDEN_2    = -WEIGHT_MAX_HIDDEN_2
WEIGHT_MIN_OUT         = -WEIGHT_MAX_OUT

OUTPUT_SCALE = 301

class NN(torch.nn.Module):

    def __init__(self):
        super().__init__()
        self.linear_white_accumulator = torch.nn.Linear(HALF_INPUT_SIZE, HALF_ACCUMULATOR_SIZE)
        self.linear_black_accumulator = torch.nn.Linear(HALF_INPUT_SIZE, HALF_ACCUMULATOR_SIZE)
        self.linear_1 = torch.nn.Linear(ACCUMULATOR_SIZE, HIDDEN_1_SIZE)
        self.linear_2 = torch.nn.Linear(HIDDEN_1_SIZE, HIDDEN_2_SIZE)
        self.linear_out = torch.nn.Linear(HIDDEN_2_SIZE, OUTPUT_SIZE)

        self._init_parameters()

    def _init_parameters(self):
        std = (2 / MAX_ACTIVE_FEATURES) ** 0.5
        torch.nn.init.normal_(self.linear_white_accumulator.weight, std=std)
        torch.nn.init.normal_(self.linear_black_accumulator.weight, std=std)
        torch.nn.init.zeros_(self.linear_white_accumulator.bias)
        torch.nn.init.zeros_(self.linear_black_accumulator.bias)

        torch.nn.init.kaiming_normal_(self.linear_1.weight)
        torch.nn.init.zeros_(self.linear_1.bias)

        torch.nn.init.kaiming_normal_(self.linear_2.weight)
        torch.nn.init.zeros_(self.linear_2.bias)

        torch.nn.init.kaiming_normal_(self.linear_out.weight)
        torch.nn.init.zeros_(self.linear_out.bias)

        with torch.no_grad():
            self.linear_1.weight.clamp_(WEIGHT_MIN_HIDDEN_1, WEIGHT_MAX_HIDDEN_1)
            self.linear_2.weight.clamp_(WEIGHT_MIN_HIDDEN_2, WEIGHT_MAX_HIDDEN_2)
            self.linear_out.weight.clamp_(WEIGHT_MIN_OUT, WEIGHT_MAX_OUT)

    def forward(self, white_features, black_features, stm):
        white_accumulator = self.linear_white_accumulator(white_features)
        black_accumulator = self.linear_black_accumulator(black_features)

        accumulator = (1 - stm) * torch.cat([white_accumulator, black_accumulator], 1) + stm * torch.cat([black_accumulator, white_accumulator], 1)

        in_1 = torch.clamp(accumulator, 0, 1)
        out_1 = self.linear_1(in_1)
        in_2 = torch.clamp(out_1, 0, 1)
        out_2 = self.linear_2(in_2)
        in_out = torch.clamp(out_2, 0, 1)
        out = self.linear_out(in_out)

        return out