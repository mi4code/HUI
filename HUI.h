#ifndef _HUI_h_
#define _HUI_h_

#ifdef __cplusplus
  #include <cstdint>
  #include <cstddef>
#else
  #include <stdint.h>
  #include <stddef.h>
  #include <stdbool.h>
#endif


#define HUI_DEBUG

#if defined(HUI_DEBUG)
  #define HUI_DEBUG_PRINT(msg) std::cout<<"\n [HUI::DEBUG] "<<msg<<" \n"
  #define HUI_DEBUG_ENABLED
#else
  #define HUI_DEBUG_PRINT(msg)
  //#undef  HUI_DEBUG_ENABLED
#endif


#if defined(_MSC_VER) && defined(_hui_backend_)
  #define MSVC_DLL __declspec(dllexport)
#else
  #define MSVC_DLL
#endif


#ifdef __cplusplus
extern "C" {
#endif


typedef void* HUI_WebView;

MSVC_DLL HUI_WebView HUI_WebView_create ();
MSVC_DLL void        HUI_WebView_destroy (HUI_WebView object);

MSVC_DLL void        HUI_WebView_load_file (HUI_WebView object, const char* file);
MSVC_DLL void        HUI_WebView_load_uri (HUI_WebView object, const char* uri);
MSVC_DLL void        HUI_WebView_load_str (HUI_WebView object, const char* str);
MSVC_DLL void        HUI_WebView_hui_tweaks (HUI_WebView object);

#ifdef __cplusplus // fixes C api, but not optimal
  MSVC_DLL const char* HUI_WebView_call_js (HUI_WebView object, const char* code, bool return_data=false);
  MSVC_DLL void        HUI_WebView_call_js_async (HUI_WebView object, const char* code, void(*return_callback)(const char*, void*), void* data=NULL); // TODO: cant do overloading
  MSVC_DLL const char* HUI_WebView_call_native (HUI_WebView object, void(*handler)(char**,int,void*), void* data, const char* process_args="function(...args_array){return args_array}"); // TODO: return value | predefined process_args for click/dragndrop/default
  MSVC_DLL const char* HUI_WebView_html_element (HUI_WebView object, const char* query, const char* property="", const char* value="");
#else
  MSVC_DLL const char* HUI_WebView_call_js (HUI_WebView object, const char* code, bool return_data/*=false*/);
  MSVC_DLL void        HUI_WebView_call_js_async (HUI_WebView object, const char* code, void(*return_callback)(const char*, void*), void* data/*=NULL*/); // TODO: cant do overloading
  MSVC_DLL const char* HUI_WebView_call_native (HUI_WebView object, void(*handler)(char**,int,void*), void* data, const char* process_args/*="function(...args_array){return args_array}"*/); // TODO: return value | predefined process_args for click/dragndrop/default
  MSVC_DLL const char* HUI_WebView_html_element (HUI_WebView object, const char* query, const char* property/*=""*/, const char* value/*=""*/);
#endif  

MSVC_DLL const char* HUI_WebView_backend_name (HUI_WebView object);
MSVC_DLL void*       HUI_WebView_backend_object (HUI_WebView object);
MSVC_DLL const char* HUI_WebView_window_type (HUI_WebView object);
MSVC_DLL void*       HUI_WebView_window_handle (HUI_WebView object);

MSVC_DLL void        HUI_WebView_handle_forever ();
MSVC_DLL void        HUI_WebView_handle_once ();
MSVC_DLL void        HUI_WebView_handle_threaded (); // TODO: not availible + solve C compatibiliy with std::thread + other languages -- see here <https://stackoverflow.com/questions/12483425/mixing-c11-stdthread-and-c-system-threads-ie-pthreads>
MSVC_DLL void        HUI_WebView_exit ();	


typedef void* HUI_WindowControls;

MSVC_DLL HUI_WindowControls HUI_WindowControls_create (void* backend, void* handle);
MSVC_DLL void        HUI_WindowControls_destroy (HUI_WindowControls object);

typedef uint8_t HUI_WindowType;
#define WT_TOPLEVEL_SSD 0
#define WT_TOPLEVEL_CSD -1
#define WT_DESKTOP_COMPONENT 1
#define WT_POPOVER 2  // TODO: what is popup and what is popover?
MSVC_DLL void        HUI_WindowControls_set_type (HUI_WindowControls object, HUI_WindowType type);
MSVC_DLL HUI_WindowType HUI_WindowControls_get_type (HUI_WindowControls object);

typedef uint8_t HUI_WindowLayer; // values match layer values in wayland layer-shell protocol
// note: WL_DEFAULT/WL_UNSET shouldnt exist (the window must be somewhere -> by default WL_TOP)
// note: window layer VS. focused/minimized/hidden - user doesnt usually change layer + the window is still in some of the layers while not visible/in some state
#define WL_BACKGROUND 0  // background (only this remains shown after user clicks "show desktop")
#define WL_BOTTOM 1      // under windows, over background
#define WL_TOP 2         // over all windows except fullscreen
#define WL_OVERLAY 3     // over everything including fullscreen
#define WL_TOPLEAST 1
#define WL_TOPMOST 3
MSVC_DLL void        HUI_WindowControls_set_layer (HUI_WindowControls object, HUI_WindowLayer layer);
MSVC_DLL HUI_WindowLayer HUI_WindowControls_get_layer (HUI_WindowControls object);

typedef int16_t HUI_WindowPixels;
typedef int8_t HUI_WindowMonitor; // TODO: multiscreen (-2) vs. unset(-1) - what should be the default behaviour for -1 - best would be primary monitor or multiscreen (now, think what when; decide monitor numbering from 0 or 1)
typedef int16_t HUI_WindowState; // most significant first (when you unset one state there is the previous all the way to floating window); bits are [state value,property set or not]  
#define WS_HIDDEN         0b11000000 // best effort possible made to make user not notice the window (may be only minimized) 
#define WS_NOT_HIDDEN     0b01000000 // WS_SHOWN
#define WS_MINIMIZED      0b00110000
#define WS_NOT_MINIMIZED  0b00010000
#define WS_FULLSCREEN     0b00001100
#define WS_NOT_FULLSCREEN 0b00000100
#define WS_MAXIMIZED      0b00000011
#define WS_NOT_MAXIMIZED  0b00000001
// note: one field should be reserved so the number is never all ones so we can have UNSET = -1 == 0b11111111.11111111 (and set bit shoul be 1 for true so &/+ operators work) 
typedef struct {
	HUI_WindowState state;
	HUI_WindowMonitor monitor;
	HUI_WindowPixels width;
	HUI_WindowPixels height;
	HUI_WindowPixels left;
	HUI_WindowPixels top;
	HUI_WindowPixels right;
	HUI_WindowPixels bottom;
} HUI_WindowGeometry; // value -1 should always mean dont set
MSVC_DLL void        HUI_WindowControls_set_geometry (HUI_WindowControls object, HUI_WindowGeometry geometry);
MSVC_DLL HUI_WindowGeometry HUI_WindowControls_get_geometry (HUI_WindowControls object);

MSVC_DLL void        HUI_WindowControls_set_opacity (HUI_WindowControls object, uint8_t opacity); // in %, default value is 100
MSVC_DLL uint8_t     HUI_WindowControls_get_opacity (HUI_WindowControls object);

MSVC_DLL void        HUI_WindowControls_set_title (HUI_WindowControls object, const char* title);
MSVC_DLL const char* HUI_WindowControls_get_title (HUI_WindowControls object);

MSVC_DLL void        HUI_WindowControls_set_id (HUI_WindowControls object, const char* id);
MSVC_DLL const char* HUI_WindowControls_get_id (HUI_WindowControls object);

MSVC_DLL void        HUI_WindowControls_set_exclusive_zone (HUI_WindowControls object, uint16_t zone);
MSVC_DLL uint16_t    HUI_WindowControls_get_exclusive_zone (HUI_WindowControls object);

MSVC_DLL void        HUI_WindowControls_set_focused (HUI_WindowControls object, bool activated);
MSVC_DLL bool        HUI_WindowControls_get_focused (HUI_WindowControls object);

typedef uint8_t HUI_WindowInputMode;
#define WIM_ALWAYS 0
#define WIM_AUTO_WINDOW 1
#define WIM_NEVER 2
// TODO: THINK ABOUT: ignore events, pass trough events, duplicate events, exclusive events VS. manual selection or window shape (most of that is not wayland compatible ~ hotkeys, window shape)
MSVC_DLL void        HUI_WindowControls_set_input_mode_keyboard (HUI_WindowControls object, HUI_WindowInputMode mode);
MSVC_DLL HUI_WindowInputMode HUI_WindowControls_get_input_mode_keyboard (HUI_WindowControls object);


#ifdef __cplusplus
}
#endif


#endif // _HUI_h_
