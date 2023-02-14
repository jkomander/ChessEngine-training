import argparse
import numpy as np
import time
import torch

from constants import*
import dataset
import model

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--train', type=str, help='Training data (.td)')
    parser.add_argument('--net', type=str)
    parser.add_argument('--num_epochs', type=int, default=100)
    parser.add_argument('--batch_size', type=int, default=1024)
    parser.add_argument('--lambda_', type=float, default=0.75)
    parser.add_argument('--lr', type=float, default=4e-2)
    parser.add_argument('--gamma', type=float, default=0.1**(1/50))
    parser.add_argument('--skip_entry_prob', type=float, default=0.75)
    args = parser.parse_args()

    config = dataset.Config(
        training_data = args.train,
        device = torch.device('cuda:0' if torch.cuda.is_available() else 'cpu'),
        num_epochs = args.num_epochs,
        batch_size = args.batch_size,
        lambda_ = args.lambda_,
        lr = args.lr,
        lr_lambda = lambda epoch : args.gamma,
        skip_entry_prob = args.skip_entry_prob
    )
    model_ = torch.load(args.net).to(config.device)
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
            torch.save(model_.cpu(), args.net)
        model_.to(config.device)
        
        end = time.time()
        
        print('Elapsed time:', end-begin)


if __name__ == '__main__':
    main()