import ctypes

lib_path = 'build/Release/training_data_loader.dll'
lib = ctypes.cdll.LoadLibrary(lib_path)

class Test(ctypes.Structure):
    _fields_ = [
        ('i', ctypes.c_int32),
        ('c', ctypes.c_char)
    ]

lib.create_test.restype = ctypes.POINTER(Test)

class TestProvider:
    def __init__(self):
        self.test = lib.create_test()
    
    def __del__(self):
        lib.destroy_test(self.test)

class SparseBatch(ctypes.Structure):
    _fields_ = [
        
    ]