import torch

class Dataset(torch.utils.data.IterableDataset):
    def __init__(self, data):
        self.data = data

    def __iter__(self):
        return iter(self.data)

def main():
    batch_size = 4

    arr = torch.arange(0, 16)
    dataset = Dataset(arr)
    loader = torch.utils.data.DataLoader(dataset,
                                         batch_size=batch_size)

    for batch in loader:
        print(batch)

if __name__ == '__main__':
    main()