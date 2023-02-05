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

lib.create_sparse_batch_stream.restype = ctypes.c_void_p
lib.create_sparse_batch_stream.argtypes = [ctypes.c_char_p, ctypes.c_size_t, ctypes.c_float]

lib.destroy_sparse_batch_stream.argtypes = [ctypes.c_void_p]

lib.next_sparse_batch.restype = ctypes.POINTER(SparseBatch)
lib.next_sparse_batch.argtypes = [ctypes.c_void_p]

lib.destroy_sparse_batch.argtypes = [ctypes.c_void_p]

class Config:
    def __init__(self, training_data, device, num_epochs, batch_size, lambda_, lr, skip_entry_prob):
        self.training_data = training_data
        self.device = device
        self.num_epochs = num_epochs
        self.batch_size = batch_size
        self.lambda_ = lambda_
        self.lr = lr
        self.skip_entry_prob = skip_entry_prob

class SparseBatchDataset(torch.utils.data.IterableDataset):
    def __init__(self, config):
        super().__init__()
        self.config = config
        self.stream = lib.create_sparse_batch_stream(
            ctypes.create_string_buffer(bytes(self.config.training_data, 'utf-8')), 
            self.config.batch_size,
            self.config.skip_entry_prob
        )
        print('Initialize dataset')

    def __iter__(self):
        return self
    
    def __next__(self):
        batch = lib.next_sparse_batch(self.stream)

        if batch:
            tensors = batch.contents.get_tensors(self.config.device)
            lib.destroy_sparse_batch(batch)
            return tensors
        
        else:
            raise StopIteration
        
    def __del__(self):
        lib.destroy_sparse_batch_stream(self.stream)
        print('Delete dataset')