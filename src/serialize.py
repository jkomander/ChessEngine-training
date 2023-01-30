import numpy as np
import torch

from constants import*
import model

class NNUE_Writer:

    def __init__(self, model_):
        self.buffer = bytearray()



def main():
    # source = 'test.pt'
    # target = 'test.nnue'
    # target = 'test.pt'
    
    # torch.manual_seed(0)
    model_ = model.NN()
    # model_ = torch.load(source)
    # torch.save(model_, target)

    # print(type(model_))
    # print(model_)
    # print(model_.state_dict())
    # -0.4292

    # if target.endwith('.nnue'):
    #     writer = NNUE_Writer(model_)
    #     with open(target, 'wb') as f:
    #         f.write(writer.buffer)
    # else:
    #     raise Exception('Invalid network output format')

if __name__ == '__main__':
    main()