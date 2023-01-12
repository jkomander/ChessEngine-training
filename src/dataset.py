import ctypes
import numpy as np
import torch

from constants import*

lib_path = 'build/Release/training_data_loader.dll'
lib = ctypes.cdll.LoadLibrary(lib_path)

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
            white_feature_indices, white_feature_values, (self.size, HALF_INPUT_SIZE)
        )
        
        black_features = torch._sparse_coo_tensor_unsafe(
            black_feature_indices, black_feature_values, (self.size, HALF_INPUT_SIZE)
        )

        white_features._coalesced_(True)
        black_features._coalesced_(True)

        return white_features, black_features, stm, score, game_result

lib.init()

lib.create_sparse_batch.restype = ctypes.POINTER(SparseBatch)
lib.create_sparse_batch.argtypes = [ctypes.c_size_t]

class SparseBatchProvider():
    def __init__(self, batch_size):
        self.sparse_batch = lib.create_sparse_batch(batch_size)

    def __del__(self):
        lib.destroy_sparse_batch(self.sparse_batch)

    def __call__(self):
        return self.sparse_batch