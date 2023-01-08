import numpy as np
import time
import torch

import dataset
import model

def main():
    HALF_INPUT_SIZE = 41920
    batch_size = 2048
    num_active_white_features_per_batch = 10
    num_active_white_features = batch_size * num_active_white_features_per_batch
    N = 100
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    torch.manual_seed(0)

    white_feature_indices = torch.empty((num_active_white_features, 2)).pin_memory().to(device=device, non_blocking=True)
    for i in range(num_active_white_features):
        white_feature_indices[i, 0] = i // num_active_white_features_per_batch
        white_feature_indices[i, 1] = 1000 * (i % num_active_white_features_per_batch)
    white_feature_indices = torch.transpose(
        white_feature_indices, 0, 1
    ).long()

    white_feature_values = torch.ones(num_active_white_features).pin_memory().to(device=device, non_blocking=True)
    white_features = torch._sparse_coo_tensor_unsafe(white_feature_indices, white_feature_values, (batch_size, HALF_INPUT_SIZE))
    white_features._coalesced_(True)

    black_features = white_features.clone().detach()

    print(white_features)

    model_ = model.NN()
    
    stm = np.empty((batch_size, 1), dtype=np.float32)
    for i in range(batch_size):
        stm[i, 0] = i % 2
    stm = torch.from_numpy(stm).pin_memory().to(device=device, non_blocking=True)

    model_.to(device)

    begin = time.time()

    for i in range(N):
        out = model_(white_features, black_features, stm)

    end = time.time()

    print(out)
    print('Elapsed time:', end-begin)

if __name__ == '__main__':
    main()

# std pytorch: 158.47129
# std engine:  148.088