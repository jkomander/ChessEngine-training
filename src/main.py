import numpy as np
import time
import torch

import dataset
import model

def main():
    batch_size = 2048
    N = 1000

    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    torch.manual_seed(0)

    sparse_batch_provider = dataset.SparseBatchProvider(batch_size)
    white_features, black_features, stm, score, game_result = sparse_batch_provider().contents.get_tensors(device)

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