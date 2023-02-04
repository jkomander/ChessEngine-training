import numpy as np
import os
import time
import torch

from constants import*
import dataset
import model

def main():

    config = dataset.Config(
        batch_size = 1024,
        device = 'cuda:0'
    )
    dataset_ = dataset.SparseBatchDataset(config)
    loader = torch.utils.data.DataLoader(dataset_, batch_size=None)

    torch.manual_seed(0)
    model_ = model.NN().to(config.device)
    optimizer = torch.optim.Adagrad(model_.parameters(), lr=5e-2)

    i = 0
    for batch in loader:
        white_features, black_features, stm, score, game_result = batch
        out = model_.forward(white_features, black_features, stm)

        optimizer.zero_grad()
        loss = model.loss_fn(out, game_result)
        loss.backward()
        optimizer.step()
        model_.zero_grad()
        model_.clamp()
        
        if (i % 100 == 0):
            print(i, out * OUTPUT_SCALE, loss, sep='\n')

        i += 1

    '''
    num_positions = 2048
    batch_size = 1024
    num_epochs = 20
    learning_rate = 100e-2

    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    torch.manual_seed(0)

    sparse_batch_provider = dataset.SparseBatchProvider(num_positions)

    train_dataset = dataset.FixedSizeDataset(
        sparse_batch_provider().contents.get_tensors(device), num_positions
    )

    train_data_loader = torch.utils.data.DataLoader(train_dataset,
                                                    batch_size=batch_size)                             

    model_ = torch.load('./default.pt').to(device)
    # model_ = model.NN().to(device)
    opt = torch.optim.SGD(model_.parameters(), lr=learning_rate)

    begin = time.time()

    for epoch in range(num_epochs):
        i = 0
        for batch in train_data_loader:
            white_features, black_features, stm, score, game_result = batch
            out = model_(white_features, black_features, stm)
            print(out * OUTPUT_SCALE)
            opt.zero_grad()
            loss = model.loss_fn(out, game_result)
            loss.backward()
            opt.step()
            model_.zero_grad()
            model_.clamp()

            print(epoch, i, loss)
            i += 1

    end = time.time()

    print('Elapsed time:', end-begin)

    # torch.save(model_.cpu(), './out.pt')
    '''

if __name__ == '__main__':
    main()