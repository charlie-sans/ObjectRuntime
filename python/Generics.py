import sys, os, inspect, importlib, types, builtins, pkgutil, importlib.util, importlib.machinery, importlib.abc, importlib.metadata
import Generictypes
from Generictypes import Value, ValueType
current_dir = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
__namespaces__ = ["System"]
class Console:
    def __init__(self):
        pass
    @staticmethod
    def WriteLine(message: str) -> None:
        """Writes a line to the console."""
        print(message)
    @staticmethod
    def ReadLine() -> str:
        """Reads a line from the console."""
        return Value(input(), ValueType.STRING)
    
Console.WriteLine("methods loaded.")