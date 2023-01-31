import torch

from constants import*
import model

class NNUE_Writer:

    def __init__(self, model_):
        self.buffer = bytearray()

        self.write_accumulator(model_)
        self.write_layer(model_.linear_1, WEIGHT_SCALE_HIDDEN_1)
        self.write_layer(model_.linear_2, WEIGHT_SCALE_HIDDEN_2)
        self.write_layer(model_.linear_out, WEIGHT_SCALE_OUT)

    # The accumulator is stored as int16 weights, int16 biases.
    def write_accumulator(self, model_):
        white_bias = model_.linear_white_accumulator.bias
        black_bias = model_.linear_black_accumulator.bias
        white_weight = model_.linear_white_accumulator.weight
        black_weight = model_.linear_black_accumulator.weight

        white_bias = white_bias.mul(INPUT_SCALE).round().to(torch.int16)
        black_bias = black_bias.mul(INPUT_SCALE).round().to(torch.int16)
        white_weight = white_weight.mul(INPUT_SCALE).round().to(torch.int16)
        black_weight = black_weight.mul(INPUT_SCALE).round().to(torch.int16)

        self.buffer.extend(white_bias.flatten().numpy().tobytes())
        self.buffer.extend(black_bias.flatten().numpy().tobytes())
        self.buffer.extend(white_weight.flatten().numpy().tobytes())
        self.buffer.extend(black_weight.flatten().numpy().tobytes())

    # Layers are stored as int16 weights, int32 biases.
    def write_layer(self, layer, weight_scale):
        bias = layer.bias
        weight = layer.weight

        bias = bias.mul(INPUT_SCALE).round().to(torch.int32)
        weight = weight.mul(weight_scale).round().to(torch.int16)

        self.buffer.extend(bias.flatten().numpy().tobytes())
        self.buffer.extend(weight.flatten().numpy().tobytes())

def main():
    # source = 'test.pt'
    target = 'test.nnue'
    # target = 'test.pt'
    
    torch.manual_seed(0)
    model_ = model.NN()
    writer = NNUE_Writer(model_)
    print(writer.buffer)
    # model_ = torch.load(source)
    # torch.save(model_, target)

    # print(type(model_))
    # print(model_)
    # print(model_.state_dict())
    # -0.4292

    # if target.endswith('.nnue'):
    #     writer = NNUE_Writer(model_)
    #     with open(target, 'wb') as f:
    #         f.write(writer.buffer)
    # else:
    #     raise Exception('Invalid network output format')

if __name__ == '__main__':
    main()