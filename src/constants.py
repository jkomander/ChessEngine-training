PIECE_INPUT_SIZE = 41916
NUM_FEATURES = 42684
PADDED_NUM_FEATURES = 42688
INPUT_HSIZE = PADDED_NUM_FEATURES
ACCUMULATOR_HSIZE = 256
INPUT_SIZE = 2*INPUT_HSIZE
ACCUMULATOR_SIZE = 2*ACCUMULATOR_HSIZE
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

BIAS_SCALE_ACCUMULATOR = 127
BIAS_SCALE_HIDDEN_1 = INPUT_SCALE * WEIGHT_SCALE_HIDDEN_1
BIAS_SCALE_HIDDEN_2 = INPUT_SCALE * WEIGHT_SCALE_HIDDEN_2
BIAS_SCALE_OUT = INPUT_SCALE * WEIGHT_SCALE_OUT

WEIGHT_MAX_HIDDEN_1    = INPUT_SCALE / WEIGHT_SCALE_HIDDEN_1
WEIGHT_MAX_HIDDEN_2    = INPUT_SCALE / WEIGHT_SCALE_HIDDEN_2
WEIGHT_MAX_OUT         = INPUT_SCALE / WEIGHT_SCALE_OUT

WEIGHT_MIN_HIDDEN_1    = -WEIGHT_MAX_HIDDEN_1
WEIGHT_MIN_HIDDEN_2    = -WEIGHT_MAX_HIDDEN_2
WEIGHT_MIN_OUT         = -WEIGHT_MAX_OUT

# score-space to float-space
OUTPUT_SCALE = 301

# loss function
LOSS_EXPONENT = 2.5

# score-space to WDL-space conversion
WDL_SCALE = 360

MATE_SCORE = 32000
MAX_EVAL_SCORE = 30000