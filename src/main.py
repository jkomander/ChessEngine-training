import numpy as np
import time
import torch

import dataset
import model

def main():
    num_positions = 2048
    batch_size = 1024
    num_epochs = 200
    learning_rate = 7e-2

    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    torch.manual_seed(0)

    sparse_batch_provider = dataset.SparseBatchProvider(num_positions)

    train_dataset = dataset.FixedSizeDataset(
        sparse_batch_provider().contents.get_tensors(device), num_positions
    )

    train_data_loader = torch.utils.data.DataLoader(train_dataset,
                                                    batch_size=batch_size)                             

    model_ = model.NN().to(device)
    opt = torch.optim.Adagrad(model_.parameters(), lr=learning_rate)

    begin = time.time()

    for epoch in range(num_epochs):
        i = 0
        for batch in train_data_loader:
            white_features, black_features, stm, score, game_result = batch
            out = model_(white_features, black_features, stm)
            opt.zero_grad()
            loss = model.loss_fn(out, game_result)
            loss.backward()
            opt.step()
            model_.zero_grad()

            print(epoch, i, loss)
            i += 1

    end = time.time()

    print('Elapsed time:', end-begin)

if __name__ == '__main__':
    main()