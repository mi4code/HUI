"""Python ctypes wrapper for the 'libHUI' shared library"""

import ctypes
import ctypes.util
import itertools
import operator
import os
import sys


# public API to export (applies for 'from HUI import *')
__all__ = [
    "WebView", "WindowControls", "WindowGeometry",
    "WT_TOPLEVEL_SSD", "WT_TOPLEVEL_CSD", "WT_DESKTOP_COMPONENT", "WT_POPOVER",
    "WL_BACKGROUND", "WL_BOTTOM", "WL_TOP", "WL_OVERLAY", "WL_TOPLEAST", "WL_TOPMOST",
    "WS_HIDDEN", "WS_NOT_HIDDEN", "WS_MINIMIZED", "WS_NOT_MINIMIZED", "WS_FULLSCREEN", "WS_NOT_FULLSCREEN", "WS_MAXIMIZED", "WS_NOT_MAXIMIZED",
    "WIM_ALWAYS", "WIM_AUTO_WINDOW", "WIM_NEVER",
]


# load LibHUI shared library
def _load_library():
    if sys.platform == "win32":
        name = "libHUI.dll"
    elif sys.platform == "darwin":
        name = "libHUI.dylib"
    else:
        name = "libHUI.so"
    
    try:
        if sys.platform == "win32":
            # return ctypes.CDLL(name, winmode=0x08) # 0x08 = 'LOAD_WITH_ALTERED_SEARCH_PATH', less safe - searches cd before %PATH%
            return ctypes.CDLL(ctypes.util.find_library("libHUI") or (_ for _ in ()).throw(ImportError("dll not found")), winmode=0x1100) # 0x1100 = 'LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS', manual lookup using %PATH% only, then load by file path 
        else:
            return ctypes.CDLL(name)
    except OSError as error:
        raise ImportError("could not load the HUI shared library %s: %s" % (name, error)) from error

_lib = _load_library()


# C API declarations
class WindowGeometry(ctypes.Structure): # struct {...}
    """Mirror of ``HUI_WindowGeometry``; the value -1 always means "don't set"."""

    _fields_ = [
        ("state", ctypes.c_int16),
        ("monitor", ctypes.c_int8),
        ("width", ctypes.c_int16),
        ("height", ctypes.c_int16),
        ("left", ctypes.c_int16),
        ("top", ctypes.c_int16),
        ("right", ctypes.c_int16),
        ("bottom", ctypes.c_int16),
    ]

    def __init__(self, state=-1, monitor=-1, width=-1, height=-1, left=-1, top=-1, right=-1, bottom=-1):
        super().__init__(state, monitor, width, height, left, top, right, bottom)

    def __repr__(self):
        return "WindowGeometry(%s)" % ", ".join("%s=%d" % (name, getattr(self, name)) for name, _ in self._fields_)

    def __eq__(self, other):
        if not isinstance(other, WindowGeometry):
            return NotImplemented
        return all(getattr(self, n) == getattr(other, n) for n, _ in self._fields_)

    __hash__ = None

_ASYNC_CB = ctypes.CFUNCTYPE(None, ctypes.c_char_p, ctypes.c_void_p) # void (*)(const char* result, void* data)
_NATIVE_CB = ctypes.CFUNCTYPE(None, ctypes.POINTER(ctypes.c_char_p), ctypes.c_int, ctypes.c_void_p) # void (*)(char** args, int len, void* data)
# note: `const char*` results are declared as c_void_p (not c_char_p) so the raw pointer survives and can be HUI_*_cstring_free'd
_SIGNATURES = [
    ("HUI_WebView_create", ctypes.c_void_p, []),
    ("HUI_WebView_destroy", None, [ctypes.c_void_p]),
    ("HUI_WebView_cstring_free", None, [ctypes.c_void_p]),
    ("HUI_WebView_load_file", None, [ctypes.c_void_p, ctypes.c_char_p]),
    ("HUI_WebView_load_uri", None, [ctypes.c_void_p, ctypes.c_char_p]),
    ("HUI_WebView_load_str", None, [ctypes.c_void_p, ctypes.c_char_p]),
    ("HUI_WebView_hui_tweaks", None, [ctypes.c_void_p]),
    ("HUI_WebView_call_js", ctypes.c_void_p, [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_bool]),
    ("HUI_WebView_call_js_async", None, [ctypes.c_void_p, ctypes.c_char_p, _ASYNC_CB, ctypes.c_void_p]),
    ("HUI_WebView_call_native", ctypes.c_void_p, [ctypes.c_void_p, _NATIVE_CB, ctypes.c_void_p, ctypes.c_char_p]),
    ("HUI_WebView_html_element", ctypes.c_void_p, [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]),
    ("HUI_WebView_backend_name", ctypes.c_void_p, [ctypes.c_void_p]),
    ("HUI_WebView_backend_object", ctypes.c_void_p, [ctypes.c_void_p]),
    ("HUI_WebView_window_type", ctypes.c_void_p, [ctypes.c_void_p]),
    ("HUI_WebView_window_handle", ctypes.c_void_p, [ctypes.c_void_p]),
    ("HUI_WebView_handle_forever", None, []),
    ("HUI_WebView_handle_once", None, []),
    ("HUI_WebView_handle_threaded", None, []),
    ("HUI_WebView_exit", None, []),

    ("HUI_WindowControls_create", ctypes.c_void_p, [ctypes.c_void_p, ctypes.c_void_p]),
    ("HUI_WindowControls_destroy", None, [ctypes.c_void_p]),
    ("HUI_WindowControls_cstring_free", None, [ctypes.c_void_p]),
    ("HUI_WindowControls_set_type", None, [ctypes.c_void_p, ctypes.c_uint8]),
    ("HUI_WindowControls_get_type", ctypes.c_uint8, [ctypes.c_void_p]),
    ("HUI_WindowControls_set_layer", None, [ctypes.c_void_p, ctypes.c_uint8]),
    ("HUI_WindowControls_get_layer", ctypes.c_uint8, [ctypes.c_void_p]),
    ("HUI_WindowControls_set_geometry", None, [ctypes.c_void_p, WindowGeometry]),
    ("HUI_WindowControls_get_geometry", WindowGeometry, [ctypes.c_void_p]),
    ("HUI_WindowControls_set_opacity", None, [ctypes.c_void_p, ctypes.c_uint8]),
    ("HUI_WindowControls_get_opacity", ctypes.c_uint8, [ctypes.c_void_p]),
    ("HUI_WindowControls_set_title", None, [ctypes.c_void_p, ctypes.c_char_p]),
    ("HUI_WindowControls_get_title", ctypes.c_void_p, [ctypes.c_void_p]),
    ("HUI_WindowControls_set_id", None, [ctypes.c_void_p, ctypes.c_char_p]),
    ("HUI_WindowControls_get_id", ctypes.c_void_p, [ctypes.c_void_p]),
    ("HUI_WindowControls_set_exclusive_zone", None, [ctypes.c_void_p, ctypes.c_uint16]),
    ("HUI_WindowControls_get_exclusive_zone", ctypes.c_uint16, [ctypes.c_void_p]),
    ("HUI_WindowControls_set_focused", None, [ctypes.c_void_p, ctypes.c_bool]),
    ("HUI_WindowControls_get_focused", ctypes.c_bool, [ctypes.c_void_p]),
    ("HUI_WindowControls_set_input_mode_keyboard", None, [ctypes.c_void_p, ctypes.c_uint8]),
    ("HUI_WindowControls_get_input_mode_keyboard", ctypes.c_uint8, [ctypes.c_void_p]),
]

def _declare(name, restype, argtypes):
    try:
        fn = getattr(_lib, name)
        fn.restype = restype
        fn.argtypes = argtypes
    except AttributeError: # missing function/symbol
        def fn(*args, **kwargs):
            raise NotImplementedError("%s is not exported by %s" % (name, _lib._name))
    return fn

_C = {name[len("HUI_"):]: _declare(name, restype, argtypes) for name, restype, argtypes in _SIGNATURES}


# type helpers

def _take_string(ptr, free):
    """Copy a string returned by libHUI into a str and release it with ``free``."""
    if not ptr:
        return ""
    try:
        return ctypes.string_at(ptr).decode("utf-8", "replace")
    finally:
        free(ptr)

def _b(value):
    """Encode a str as UTF-8 bytes for a C ``const char*`` argument; bytes and bytearray are copied to bytes."""
    if isinstance(value, str):
        return value.encode("utf-8")
    if isinstance(value, (bytes, bytearray)):
        return bytes(value)
    raise TypeError("expected str, got %s" % type(value).__name__)

def _int(value, lo, hi, what):
    """Return ``value`` as an int if it is an integer (has ``__index__``) in [lo, hi]."""
    try:
        n = operator.index(value)
    except TypeError:
        raise TypeError("%s must be an integer, got %s" % (what, type(value).__name__)) from None
    if not lo <= n <= hi:
        raise OverflowError("%s must be in range [%d, %d], got %d" % (what, lo, hi, n))
    return n

_CapsuleGetName = ctypes.PYFUNCTYPE(ctypes.c_char_p, ctypes.py_object)(("PyCapsule_GetName", ctypes.pythonapi))
_CapsuleGetPointer = ctypes.PYFUNCTYPE(ctypes.c_void_p, ctypes.py_object, ctypes.c_char_p)(("PyCapsule_GetPointer", ctypes.pythonapi))
def _void_p(value):
    """Accept what the pybind11 ``void*`` caster accepted: None or a capsule (plus ints)."""
    if value is None or isinstance(value, int):
        return value
    if isinstance(value, ctypes.c_void_p):
        return value.value
    if type(value).__name__ == "PyCapsule":
        return _CapsuleGetPointer(value, _CapsuleGetName(value))
    return value  # let ctypes try (pointers, objects with _as_parameter_, ...)


## WebView

# callback plumbing (C gets one fixed trampoline per kind and an integer token, which maps back to the Python callable, so nothing is garbage collected while C holds it)

_callbacks = {}
_tokens = itertools.count(1)

def _register(fn):
    token = next(_tokens)
    _callbacks[token] = fn
    return token

@_ASYNC_CB
def _async_trampoline(result, token):
    fn = _callbacks.pop(token, None)  # one-shot (called once)
    if fn is None:
        return
    fn("" if result is None else result.decode("utf-8", "replace"))

@_NATIVE_CB
def _native_trampoline(argv, argc, token):
    fn = _callbacks.get(token)  # persistent (may be called any number of times)
    if fn is None:
        return
    fn([("" if argv[i] is None else argv[i].decode("utf-8", "replace")) for i in range(argc)])

class WebView:
    def __init__(self):
        handle = _C["WebView_create"]()
        if not handle:
            raise RuntimeError("HUI_WebView_create() returned NULL")
        self._handle = handle
        self._native_tokens = []

    def _h(self):
        handle = self._handle
        if handle is None:
            raise RuntimeError("WebView has already been destroyed")
        return handle

    def _str(self, ptr):
        return _take_string(ptr, _C["WebView_cstring_free"])

    def _destroy(self):
        handle = getattr(self, "_handle", None)
        if handle is None:
            return
        self._handle = None
        tokens = self.__dict__.get("_native_tokens", ())
        try:
            _C["WebView_destroy"](handle)
        finally:
            for token in tokens:
                _callbacks.pop(token, None)

    def __enter__(self):
        return self

    def __exit__(self, exc_type=None, exc_value=None, traceback_=None):
        self._destroy()

    def __del__(self):
        try:
            self._destroy()
        except Exception:
            pass

    def load_file(self, file):
        _C["WebView_load_file"](self._h(), _b(os.fspath(file) if isinstance(file, os.PathLike) else file))

    def load_uri(self, uri):
        _C["WebView_load_uri"](self._h(), _b(uri))

    def load_str(self, str):
        _C["WebView_load_str"](self._h(), _b(str))

    def hui_tweaks(self):
        _C["WebView_hui_tweaks"](self._h())

    def call_js(self, code, return_data=False):
        """ 'call_js(code, [return_data=False]) -> None' or 'call_js(code, return_data=True) -> str' or 'call_js(code, callback) -> None' """
        handle, code_b = self._h(), _b(code)
        if callable(return_data):
            token = _register(return_data)
            try:
                _C["WebView_call_js_async"](handle, code_b, _async_trampoline, token)
            except BaseException:
                _callbacks.pop(token, None)
                raise
        elif return_data == True:
            return self._str(_C["WebView_call_js"](handle, code_b, bool(return_data)))
        elif return_data == False:
            self._str(_C["WebView_call_js"](handle, code_b, bool(return_data)))
        else:
            raise TypeError("Invalid return mode option.")

    def call_native(self, handler, process_args="function(...args_array){return args_array}"):
        if not callable(handler):
            raise TypeError("handler must be callable")
        handle, process_b = self._h(), _b(process_args)
        token = _register(handler)
        try:
            result = _C["WebView_call_native"](handle, _native_trampoline, token, process_b)
        except BaseException:
            _callbacks.pop(token, None)
            raise
        self._native_tokens.append(token)
        return self._str(result)

    def html_element(self, query, property="", value=""):
        return self._str(_C["WebView_html_element"](self._h(), _b(query), _b(property), _b(value)))

    def backend_name(self):
        return self._str(_C["WebView_backend_name"](self._h()))

    def backend_object(self):
        return _C["WebView_backend_object"](self._h())

    def window_type(self):
        return self._str(_C["WebView_window_type"](self._h()))

    def window_handle(self):
        return _C["WebView_window_handle"](self._h())

    @staticmethod
    def handle_forever():
        _C["WebView_handle_forever"]()

    @staticmethod
    def handle_once():
        _C["WebView_handle_once"]()

    @staticmethod
    def handle_threaded():
        _C["WebView_handle_threaded"]()

    @staticmethod
    def exit():
        _C["WebView_exit"]()


## WindowControls

# constants from HUI.h

WT_TOPLEVEL_SSD = 0
WT_TOPLEVEL_CSD = -1
WT_DESKTOP_COMPONENT = 1
WT_POPOVER = 2

WL_BACKGROUND = 0
WL_BOTTOM = 1
WL_TOP = 2
WL_OVERLAY = 3
WL_TOPLEAST = 1
WL_TOPMOST = 3

WS_HIDDEN = 0b11000000
WS_NOT_HIDDEN = 0b01000000
WS_MINIMIZED = 0b00110000
WS_NOT_MINIMIZED = 0b00010000
WS_FULLSCREEN = 0b00001100
WS_NOT_FULLSCREEN = 0b00000100
WS_MAXIMIZED = 0b00000011
WS_NOT_MAXIMIZED = 0b00000001

WIM_ALWAYS = 0
WIM_AUTO_WINDOW = 1
WIM_NEVER = 2

class WindowControls:
    def __init__(self, backend, handle):
        ptr = _C["WindowControls_create"](_void_p(backend), _void_p(handle))
        if not ptr:
            raise RuntimeError("HUI_WindowControls_create() returned NULL")
        self._handle = ptr

    def _h(self):
        handle = self._handle
        if handle is None:
            raise RuntimeError("WindowControls has already been destroyed")
        return handle

    def _str(self, ptr):
        return _take_string(ptr, _C["WindowControls_cstring_free"])

    def _destroy(self):
        handle = getattr(self, "_handle", None)
        if handle is None:
            return
        self._handle = None
        _C["WindowControls_destroy"](handle)

    def __del__(self):
        try:
            self._destroy()
        except Exception:
            pass

    def set_type(self, type):
        type = _int(type, -1, 255, "type")
        # WT_TOPLEVEL_CSD is -1, which the C API stores in a uint8_t
        _C["WindowControls_set_type"](self._h(), 0xFF if type == -1 else type)

    def get_type(self):
        value = _C["WindowControls_get_type"](self._h())
        return -1 if value == 0xFF else value

    def set_layer(self, layer):
        _C["WindowControls_set_layer"](self._h(), _int(layer, 0, 255, "layer"))

    def get_layer(self):
        return _C["WindowControls_get_layer"](self._h())

    def set_geometry(self, geometry):
    
        def _coerce_geometry(geometry):
            if isinstance(geometry, WindowGeometry):
                return geometry
            if isinstance(geometry, dict):
                return WindowGeometry(**geometry)
            if isinstance(geometry, (tuple, list)):
                return WindowGeometry(*geometry)
            raise TypeError("geometry must be a WindowGeometry, dict or tuple, not %s" % type(geometry).__name__)
            
        _C["WindowControls_set_geometry"](self._h(), _coerce_geometry(geometry))

    def get_geometry(self):
        return _C["WindowControls_get_geometry"](self._h())

    def set_opacity(self, opacity):
        _C["WindowControls_set_opacity"](self._h(), _int(opacity, 0, 255, "opacity"))

    def get_opacity(self):
        return _C["WindowControls_get_opacity"](self._h())

    def set_id(self, id):
        _C["WindowControls_set_id"](self._h(), _b(id))

    def get_id(self):
        return self._str(_C["WindowControls_get_id"](self._h()))

    def set_title(self, title):
        _C["WindowControls_set_title"](self._h(), _b(title))

    def get_title(self):
        return self._str(_C["WindowControls_get_title"](self._h()))

    def set_exclusive_zone(self, zone):
        # C API takes uint16_t, the C++ API int16_t: pass the two's complement bits
        _C["WindowControls_set_exclusive_zone"](self._h(), _int(zone, -32768, 32767, "zone") & 0xFFFF)

    def get_exclusive_zone(self):
        value = _C["WindowControls_get_exclusive_zone"](self._h())
        return value - 0x10000 if value >= 0x8000 else value

    def set_focused(self, activated):
        _C["WindowControls_set_focused"](self._h(), bool(activated))

    def get_focused(self):
        return bool(_C["WindowControls_get_focused"](self._h()))

    def set_input_mode_keyboard(self, mode):
        _C["WindowControls_set_input_mode_keyboard"](self._h(), _int(mode, 0, 255, "mode"))

    def get_input_mode_keyboard(self):
        return _C["WindowControls_get_input_mode_keyboard"](self._h())
