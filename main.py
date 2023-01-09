import numpy as np
import time
import torch

import dataset
import model

def main():
    HALF_INPUT_SIZE = 41920
    batch_size = 2048
    num_active_white_features = batch_size * 10
    num_active_black_features = num_active_white_features
    N = 1000
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    torch.manual_seed(0)

    sparse_batch_provider = dataset.SparseBatchProvider()
    white_features, black_features, stm, score = sparse_batch_provider().contents.get_tensor(device)

    model_ = model.NN().to(device)

    begin = time.time()

    for i in range(N):
        out = model_(white_features, black_features, stm)

    end = time.time()

    print(white_features)
    print(out)
    print('Elapsed time:', end-begin)

if __name__ == '__main__':
    main()

# std pytorch: 158.47129
# std engine:  148.088