import numpy as np
import os
import time
import torch

from constants import*
import dataset
import model

def main():
    config = dataset.Config(
        training_data = os.path.abspath('games.td'),
        device = torch.device('cuda:0' if torch.cuda.is_available() else 'cpu'),
        num_epochs = 3000000,
        batch_size = 1024,
        lambda_ = 0.95,
        lr = 1e-3,
        lr_lambda = lambda epoch : 0.1 ** (1/300),
        # lr_lambda = lambda epoch : 1,
        skip_entry_prob = 0.75
    )
    model_ = torch.load('./temp.pt').to(config.device)
    optimizer = torch.optim.Adagrad(model_.parameters(), config.lr)
    scheduler = torch.optim.lr_scheduler.MultiplicativeLR(optimizer, lr_lambda=config.lr_lambda, verbose=True)

    for epoch in range(config.num_epochs):
        begin = time.time()
    
        dataset_ = dataset.SparseBatchDataset(config)
        loader = torch.utils.data.DataLoader(dataset_, batch_size=None)

        i = 0
        for batch in loader:
            white_features, black_features, stm, score, game_result = batch
            out = model_.forward(white_features, black_features, stm)

            optimizer.zero_grad()
            loss = model.loss_fn(out, score, game_result, config.lambda_)
            loss.backward()
            optimizer.step()
            model_.zero_grad()
            model_.clamp()
            
            if i % 100 == 0:
                print(epoch, i, game_result, score, out * OUTPUT_SCALE, loss, sep='\n')

            i += 1

        scheduler.step()

        if (epoch+1) % 1 == 0:
            torch.save(model_.cpu(), './temp.pt')
        model_.to(config.device)
        
        end = time.time()
        
        print('Elapsed time:', end-begin)


if __name__ == '__main__':
    main()