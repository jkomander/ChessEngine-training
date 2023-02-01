import argparse
import functools
import numpy as np
import operator
import torch

from constants import*
import model

class NNUE_Writer:
    def __init__(self, model_):
        self.buffer = bytearray()

        self.write_accumulator(model_)
        self.write_layer(model_.linear_1, BIAS_SCALE_HIDDEN_1, WEIGHT_SCALE_HIDDEN_1)
        self.write_layer(model_.linear_2, BIAS_SCALE_HIDDEN_2, WEIGHT_SCALE_HIDDEN_2)
        self.write_layer(model_.linear_out, BIAS_SCALE_OUT, WEIGHT_SCALE_OUT)

    # The accumulator is stored as int16 weights, int16 biases.
    def write_accumulator(self, model_):
        white_bias = model_.linear_white_accumulator.bias
        black_bias = model_.linear_black_accumulator.bias
        white_weight = model_.linear_white_accumulator.weight
        black_weight = model_.linear_black_accumulator.weight

        white_bias = white_bias.mul(BIAS_SCALE_ACCUMULATOR).round().to(torch.int16)
        black_bias = black_bias.mul(BIAS_SCALE_ACCUMULATOR).round().to(torch.int16)
        white_weight = white_weight.mul(WEIGHT_SCALE_ACCUMULATOR).round().to(torch.int16)
        black_weight = black_weight.mul(WEIGHT_SCALE_ACCUMULATOR).round().to(torch.int16)

        self.buffer.extend(white_bias.flatten().numpy().tobytes())
        self.buffer.extend(black_bias.flatten().numpy().tobytes())
        self.buffer.extend(white_weight.flatten().numpy().tobytes())
        self.buffer.extend(black_weight.flatten().numpy().tobytes())

    # Layers are stored as int16 weights, int32 biases.
    def write_layer(self, layer, bias_scale, weight_scale):
        bias = layer.bias
        weight = layer.weight

        bias = bias.mul(bias_scale).round().to(torch.int32)
        weight = weight.mul(weight_scale).round().to(torch.int16)

        self.buffer.extend(bias.flatten().numpy().tobytes())
        self.buffer.extend(weight.flatten().numpy().tobytes())    

class NNUE_Reader:
    def __init__(self, f):
        self.f = f
        self.model_ = model.NN()
        self.read_accumulator()
        self.read_layer(self.model_.linear_1, BIAS_SCALE_HIDDEN_1, WEIGHT_SCALE_HIDDEN_1)
        self.read_layer(self.model_.linear_2, BIAS_SCALE_HIDDEN_2, WEIGHT_SCALE_HIDDEN_2)
        self.read_layer(self.model_.linear_out, BIAS_SCALE_OUT, WEIGHT_SCALE_OUT)

    def tensor(self, shape, dtype):
        tensor = np.fromfile(self.f, dtype, functools.reduce(operator.mul, shape, 1))
        tensor = torch.from_numpy(tensor.astype(np.float32))
        tensor = tensor.reshape(shape)
        return tensor

    def read_accumulator(self):
        white_bias = self.tensor(self.model_.linear_white_accumulator.bias.shape, np.int16)
        black_bias = self.tensor(self.model_.linear_black_accumulator.bias.shape, np.int16) 
        white_weight = self.tensor(self.model_.linear_white_accumulator.weight.shape, np.int16)
        black_weight = self.tensor(self.model_.linear_black_accumulator.weight.shape, np.int16)

        self.model_.linear_white_accumulator.bias.data = white_bias.div(BIAS_SCALE_ACCUMULATOR)
        self.model_.linear_black_accumulator.bias.data = black_bias.div(BIAS_SCALE_ACCUMULATOR)
        self.model_.linear_white_accumulator.weight.data = white_weight.div(WEIGHT_SCALE_ACCUMULATOR)
        self.model_.linear_black_accumulator.weight.data = black_weight.div(WEIGHT_SCALE_ACCUMULATOR)

    def read_layer(self, layer, bias_scaling, weight_scaling):
        layer.bias.data = self.tensor(layer.bias.shape, np.int32).div(bias_scaling)
        layer.weight.data = self.tensor(layer.weight.shape, np.int16).div(weight_scaling)

def main():
    parser = argparse.ArgumentParser(description='Converts files between pt and nnue format')
    parser.add_argument("source", help='Source file (can be .pt or .nnue)')
    parser.add_argument("target", help='Target file (can be .pt or .nnue)')
    
    args =  parser.parse_args()
    print('Converting {} to {}'.format(args.source, args.target))

    if args.source.endswith('.nnue'):
        with open(args.source, 'rb') as f:
            reader = NNUE_Reader(f)
        model_ = reader.model_

    elif args.source.endswith('.pt'):
        model_ = torch.load(args.source)

    else:
        raise Exception('Invalid network output format')

    if args.target.endswith('.nnue'):
        writer = NNUE_Writer(model_)
        with open(args.target, 'wb') as f:
            f.write(writer.buffer)

    elif args.target.endswith('.pt'):
        torch.save(model_, args.target)

    else:
        raise Exception('Invalid network output format')

if __name__ == '__main__':
    main()