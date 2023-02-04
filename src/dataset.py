import ctypes
import glob
import numpy as np
import os
import sys
import torch

from constants import*

# Default Visual Studio path
libpath = os.path.abspath('build/Release/training_data_loader.dll')

if not os.path.isfile(libpath):
    local_libpath = [n for n in glob.glob('./*training_data_loader.*') if n.endswith('.so') or n.endswith('.dll') or n.endswith('.dylib')]
    if not local_libpath:
        print('Cannot find training_data_loader shared library.')
        sys.exit(1)
    libpath = os.path.abspath(local_libpath[0])

lib = ctypes.cdll.LoadLibrary(libpath)
lib.init()

class SparseBatch(ctypes.Structure):
    _fields_ = [
        ('size', ctypes.c_int64),
        ('num_active_white_features', ctypes.c_int64),
        ('num_active_black_features', ctypes.c_int64),
        ('stm', ctypes.POINTER(ctypes.c_float)),
        ('score', ctypes.POINTER(ctypes.c_float)),
        ('game_result', ctypes.POINTER(ctypes.c_float)),
        ('white_feature_indices', ctypes.POINTER(ctypes.c_int64)),
        ('black_feature_indices', ctypes.POINTER(ctypes.c_int64)),
        ('white_feature_values', ctypes.POINTER(ctypes.c_float)),
        ('black_feature_values', ctypes.POINTER(ctypes.c_float))
    ]

    def get_tensors(self, device):
        stm = torch.from_numpy(
            np.ctypeslib.as_array(self.stm, shape=(self.size, 1))
        ).pin_memory().to(device=device, non_blocking=True)

        score = torch.from_numpy(
            np.ctypeslib.as_array(self.score, shape=(self.size, 1))
        ).pin_memory().to(device=device, non_blocking=True)

        game_result = torch.from_numpy(
            np.ctypeslib.as_array(self.game_result, shape=(self.size, 1))
        ).pin_memory().to(device=device, non_blocking=True)

        white_feature_indices = torch.transpose(
            torch.from_numpy(
                np.ctypeslib.as_array(self.white_feature_indices, shape=(self.num_active_white_features, 2))
            ).pin_memory().to(device=device, non_blocking=True), 
            0, 1
        ).long()
        
        black_feature_indices = torch.transpose(
            torch.from_numpy(
                np.ctypeslib.as_array(self.black_feature_indices, shape=(self.num_active_black_features, 2))
            ).pin_memory().to(device=device, non_blocking=True), 
            0, 1
        ).long()

        white_feature_values = torch.from_numpy(
            np.ctypeslib.as_array(self.white_feature_values, shape=(self.num_active_white_features,))
        ).pin_memory().to(device=device, non_blocking=True)

        black_feature_values = torch.from_numpy(
            np.ctypeslib.as_array(self.black_feature_values, shape=(self.num_active_black_features,))
        ).pin_memory().to(device=device, non_blocking=True)

        white_features = torch._sparse_coo_tensor_unsafe(
            white_feature_indices, white_feature_values, (self.size, INPUT_HSIZE)
        )
        
        black_features = torch._sparse_coo_tensor_unsafe(
            black_feature_indices, black_feature_values, (self.size, INPUT_HSIZE)
        )

        white_features._coalesced_(True)
        black_features._coalesced_(True)

        return white_features, black_features, stm, score, game_result

lib.create_sparse_batch.restype = ctypes.POINTER(SparseBatch)
lib.create_sparse_batch.argtypes = [ctypes.c_size_t]

class SparseBatchProvider():
    def __init__(self, batch_size):
        self.sparse_batch = lib.create_sparse_batch(batch_size)

    def __del__(self):
        lib.destroy_sparse_batch(self.sparse_batch)

    def __call__(self):
        return self.sparse_batch

class FixedSizeDataset(torch.utils.data.Dataset):
    def __init__(self, data, size):
        super().__init__()
        self.data = data
        self.size = size
    
    def __len__(self):
        return self.size

    def __getitem__(self, index):
        return self.data[0][index], self.data[1][index], self.data[2][index], self.data[3][index], self.data[4][index]

class Config:
    def __init__(self, batch_size, device):
        self.batch_size = batch_size
        self.device = device

class SparseBatchDataset(torch.utils.data.IterableDataset):
    def __init__(self, config):
        super().__init__()
        self.config = config

    def __iter__(self):
        return self
    
    def __next__(self):
        sparse_batch_provider = SparseBatchProvider(self.config.batch_size)

        if sparse_batch_provider:
            tensors = sparse_batch_provider().contents.get_tensors(self.config.device)
            return tensors
        
        else:
            raise StopIteration