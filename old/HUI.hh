#if !defined(HUI_HH)
 #define HUI_HH

#include <cstdint>
#include <iostream>
#include <vector>
#include <functional>
#include <thread>


//#if defined(HUI_BUILD_CPP)
// #include "hui_new.cc"
//#endif


/*
before 0.0.24:
all in one file
window base is "master" class
the controls and content clases should be created for the base with their own code 

after 0.0.24:
WindowBase with content is discontinued - replaced with WindowBase2 for embeding windows
WebviewContent becomes window itself
the controls are universal for any window
*/

 /*
	this file is the main (and only) HUI include
	you dont need to include anything else as this file already does it
	
	HUI project uses its own datatypes but you dont have to - whole api is based on standard C++ types 
	
	this file contains multiple bindings and backends - see HUI_* defines below for building options
	  <no defines> - building binary with linking to HUI
	  HUI_BUILD_CPP - building HUI library (static or dynamic) or using HUI as header only library
	  HUI_BUILD_PYTHON - building HUI as python module (you should link it aginst libHUI.so or add HUI_BUILD_CPP to include backend)

	  TODO: HUI_BUILD_C
	  TODO: select backend + allow non C++ backends (rust) + code for binding C<-->C++
	  TODO: Windows dll (what about win dll export/import directives?)
	  TODO: build python binding with MSVC linking to libHUI.dll built with msys2 mingw
	  
BINDINGS:
 C HUI.h + libHUI.so
 C++ HUI.hh + libHUI.so
 Python + HUI.*.{pyd,so}
	  
 */

namespace HUI { 


typedef int8_t WindowType;
const WindowType
	WT_TOPLEVEL=0,     // TODO: auto-neogatiate decoration profile and when returning get_type return _CSD or _SSD
	WT_TOPLEVEL_SSD=0, // server side decorations (or none if compositor doesnt offer them)
	WT_TOPLEVEL_CSD=1, // client side decorations (custom - no decorations unless you add them)
	WT_DESKTOP_COMPONENT=2,
	WT_POPOVER=3;
	// TODO: number definitions are not fixed yet
	// TODO: or (WT_NO_TITLEBAR | WT_NO_FRAME) == WT_DESKTOP_COMPONENT
	// the mini corner decorations will be decided by window manager 
	// note: wayland protocol xdg-decoration provides only two options for decorations: CSD=1 and SSD=2 
	// note: other platforms have usually these options: TITLEBAR yes/no, FRAME yes/no + TASKBAR yes/no + POPOVER yes/no
	// old notes from WindowDecorations:
	/*enum WindowDecorations { // currently unused // this library is server side decorations only for now + should be decided by wm not by the application // TODO: implement <https://docs.gtk.org/gtk3/method.Window.set_titlebar.html>
		//WD_YES  // decoration scheme should be decided by the compositor
		WD_DEFAULT, // should be yes
		
		WD_SERVER, // titlebar present + frame present // can use gtk-nocsd project or GTK_CSD=0 environ
		WD_CLIENT, // no titlebar + frame present // is ready for custom decorations - doesnt have titlebar but still has frame (should really have? - in case you want custom frame) and is dragable (by custom area)
		WD_NONE, // no titlebar + no frame // completely frameless (on wayland is layer shell window), used for popups, currently called unmanaged // does no frame mean it cant be resized?
		
		//WD_CLIENT_FULL, // bar (=topmost, cmd, rotate, ... ~delivered by HUI)
		//WD_CLIENT_MINI, // in corner (semitransparent, shown on hover ~delivered by HUI) // buttons in specified corner of the window BUT no titlebar (hybrid server+client)
	};*/


typedef int8_t WindowLayer;
const WindowLayer
  // toplevel windows:
	WL_TOPLEAST   =1,
    //WL_TOP        =2,
	WL_TOPMOST    =3,
  // layer shell windows (values match the layer shell specifications):
	WL_BACKGROUND =0, // background (only this remains shown after user clicks "show desktop")
	WL_BOTTOM     =1, // under windows, over background
	WL_TOP        =2, // over all windows except fullscreen
	WL_OVERLAY    =3; // over everything including fullscreen
  // TODO: WL_UNSET=-1 (hould be there, so every property has -1 with unset meaning)
  // note: window layer VS. focused/minimize -- user doesnt change layer (usually) BUT user activates/minimizes the window
  // note: when needed to add support for stacking position in as window count, it should be in negative numbers
  // note: WL_DEFAULT shouldnt exist (the window must be somewhere -> same as WL_TOP)
  // note: hidden cant be part of this because after unhide the window should be shown somewhere (same layer as before hide)


typedef int8_t WindowInputMode;
const WindowInputMode
	WIM_ALWAYS=1,      // keyboard: (only-focused) the only existing window you can type in              // mouse: default for mouse
	WIM_AUTO_WINDOW=2, // keyboard: default for keyboard                                  // mouse: is same as _ALWAYS
	//WIM_AUTO_ELEMENT,  // TODO: (selective mode)  keyboard: window focused only when text input element clicked +  mouse: click-trough areas 
	WIM_NEVER=0;       // keyboard: (only-unfocused) the window cant get focused=activated so you cant type into it  // mouse: click trough (OR ignore click - but it can be done with css - same with keyboard)
  // TODO: duplicate mode (click trough but receive clicks too; keyboard input even if not focused)
  // TODO: selective mode (click trough area or select elements that are clickable, keyboard focus only when one of selected elements focused) -- can be achieved using some intensive mouse position pooling and switching between WIM_ALWAYS and WIM_NEVER
  // TODO: ignore input but capture it
  // TODO: window shapes (using selective mode) - not implemented yet (best would be if windows managers handled window shapes themselves + rectangle is best option anyway)
  /*
  // OLD: alternative names + functions
   WE_TAKE, // default - window takes all events (when active)
   WE_PASS_THROUGH, // dont activate and pass events to underlaying window
   WE_GRAB, // window consumes all input and doesnt allow other windows to get it
   WE_DUPLICATE, // events go to the window but also are copied to underlaying window
   WE_SELECTIVE, // ?hanled by the user - allow click on selected elements / activate window only when user clicked into the input area
   WE_BLOCK, // take events but act like they never happened (can be done trough html)
  */


typedef int8_t WindowState; // odd bit is ?set, even bit is value ;; 0 means unset
const WindowState
	WS_MAXIMIZED = 0b11000000,
	WS_N_MAXIMIZED = 0b10000000,
	WS_FULLSCREEN = 0b00110000,
	WS_N_FULLSCREEN = 0b00100000,
	WS_MINIMIZED = 0b00001100,
	WS_N_MINIMIZED = 0b00001000;
	// note: https://developer.mozilla.org/en-US/docs/Mozilla/Add-ons/WebExtensions/API/windows/WindowState
	// note: this is not how it works: https://learn.microsoft.com/en-us/dotnet/api/system.windows.windowstate?view=windowsdesktop-8.0
	// note: some ideas (about non standart wms): https://www.reddit.com/r/linux/comments/16vciex/lets_make_wayland_have_support_for_window/


typedef int16_t WindowPixels; // = 4x 8K screen = 32K screen  // pixels after fractional scale (not real pixels)
typedef int8_t WindowScreen; // = 127 screens
typedef int16_t Pixels; // = 4x 8K screen = 32K screen  // pixels after fractional scale (not real pixels)
typedef int8_t Screen; // = 127 monitors (number) + full workspace = screen (all monitors)

//const Screen S_ALL = -10; // -10~all -1~primary // NOT SUPPORTED YET  // issue with wayfire where layer window is shown only on 1 screen at time 

//DEFAULT = -1 (~better not -1 == 65535) = dont config ...should exist for all config options (but still with some exceptions)   --there shouldnt be such thing as default not saying what will happen BUT we sometimes need to config only some settings


	

	
	



#if defined(_WIN32)
	typedef HWND Handle;
#else 
	typedef void* Handle;
#endif

struct Rect {
	
	Rect(){
		
	}
	Rect(int16_t x, int16_t y, uint16_t w, uint16_t h){
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
	}
	Rect(int16_t x, int16_t y, uint16_t w, uint16_t h, Rect relative){
		this->x = x + relative.x;
		this->y = y + relative.y;
		this->w = w;
		this->h = h;
	}
	//Rect(left,top,right,bottom, relative)
	//Rect(left,top,right,bottom,w,h, relative)
	// ?rect to anchors
	
	
	int16_t x,y;
	uint16_t w,h;
};


class Window;

void handle_forever();
void handle_once();
void exit();
// TODO: whole ui will come from HUI::App -> HUI::App.new_window(), HUI::App.handle()

class App { // common info for all windows/whole binary + maybe quality of dev life simplifications (integration, preferences, notifications, ...)
  public:
    App(const char* name = NULL, const char* icon = NULL, const char* binary = NULL);
	
	Window new_window(); // TODO: or something like void own_window(Window);   what about Window base classes? what about using this inside Window constructor?

  private:
    const char* name;  // full application name
    const char* binary;  // application binary name   TODO: with or without path? (there may be multiple versions/instances running)
    const char* icon;  // app icon - used for panel icon and by default also for windows
};

class WebviewContent { // the html view (the main component of the ui, but not the only one in the future) -- content will be window=widget embedable into WindowBase
  public:
	WebviewContent ();
	~WebviewContent ();
	
	void content (std::string file);
	std::string call_js(std::string code, bool return_data=false);
	std::string call_cpp(std::function<void(std::vector<std::string>)> handler, const char* process_args="function(...args_array){return args_array}");  // registers callback  // TODO: remove/rename?
	void* backend_object ();
	Handle window_handle ();
	
	std::string to_js (std::function<void(std::vector<std::string>)> value, const char* process_args="function(...args_array){return args_array}");
	std::string to_js (std::string value);
	std::string to_js (long value);
	std::string to_js (float value);
	
	void js_set_value (const char* code, std::string value);
	void js_set_value (const char* query, const char* property, std::string value, uint16_t index = -1);

	std::string js_get_value (const char* code);
	std::string js_get_value (const char* query, const char* property, uint16_t index = 0);
	
	// TODO: js values escaping - set vs. get vs. call_js + returning functions
	// TODO:  AUTO get_value<typename> OR create new struct (that can also support object.properties and function calls)
	// TODO: to_js should be external function so the api is simpler
	
  private:
  #if defined(HUI_BACKEND_GTK3)
	//GtkWidget* window = NULL;
	//GtkWidget* webview = NULL;
	//GtkWidget* content = NULL; // should be some kind of structure
	void* window = NULL;
	void* webview = NULL;
  #elif defined(HUI_BACKEND_QT5)
	//QWidget* window;
	//QWebView* webview;
	void* window = NULL;
	void* webview = NULL;
  #else
	//std::any webview; // doesnt work well with pointer
	//std::unique_ptr<auto> webview; // needs a type
	void* instance = NULL;
  #endif
};

Handle ProcessWindow (const char* run);

class WindowBase2 {
public:
    WindowBase2();
    ~WindowBase2();
	
	void embed (Handle child, Rect(*positioner)(Rect) = [](Rect rect){return rect;}); // TODO: lambda with capture
	void detach (Handle handle);
    
    Handle window_handle(); // get the toplevel window handle
    
    void run_blocking(); // stand-alone  // TODO: rethink so multiple instances possible
    void run_nonblocking(); // should be inside some kind of loop
    std::thread run_inthread(); // in thread non blocking (or blocking if used as .run_inthread().join())

private:
  #if defined(HUI_BACKEND_WINDOWS)
    Handle hwndParent = NULL;
	
	struct StructWindows {Handle handle; Rect(*positioner)(Rect);}; // TODO: focus (its kinda random currently) and z_order (currently defined by the order of embeding)
	std::vector<StructWindows> windows;
	
    HINSTANCE hInst;
  #else
	void* instance = NULL;
  #endif
};

class WindowControls { // should be built in the most native way (to be able to control any window, not just the one created by this toolkit)
  public:
	//GtkWidget* window = NULL;
	//QWidget* window;
	//void* window = NULL;
	void* window;
	
	#if defined(HUI_BACKEND_WINDOWS)
	Handle handle; // TODO: auto or std::any or void* or std::variant or define
	#endif
	
	WindowControls(); // TODO: its possible to select backend ... environ GDK_BACKEND="wayland/x11"
	// TODO: add export id/handle to WindowBase so it can be imported in WindowControls
	// TODO: add callback -> set_,get_,watch_/on_changed_  (dont forget to maintain C compatibility) 
	
	void use (WebviewContent* _window); // TODO: use handle
	
	/*
		Completely shows/hides the window so it can be manipulated without letting user see.
	*/
	void show ();
	void hide ();


	/*
		Sets/gets window title.
		---
		OK
	*/
	void set_title(const char* title);
	const char* get_title();
	
	/*
		Sets/gets Wayland app_id / layer-shell namespace / x11 startup id.
		Meant to distinguish windows belonging to the same app.
		---
		TODO: complete GTK3
	*/
	void set_id(const char* id);
	const char* get_id();
	
	/*
		Sets/gets window type.
		---
		TODO: what are the ways to create frameless window? (frameless vs. client-decorated vs. no-titlebar)
	*/
	void set_type (WindowType type);
	WindowType get_type ();
	
	/*
		Sets/gets the window geometry as the full window definition in the 2D screen space.
		`left` is the same as `x` coordinate and `top` is the `y` coordinate.
		Position overrides size in case of over-definition.
		As an return value, you can expect any value format that defines the position.
		---
		TODO: ... Screen relative = (screen number or window pointer)/SCREEN_DEFAULT=-1/SCREEN_MULTISCREEN=-2/SCREEN_PRIMARY=-3, ?flags) 
		TODO: tiling support - add support for values in % of relative or add function relative_to_rect
		TODO: decorated (can we access it?) vs. non-decorated window size problem: this is not meant for decorated windows (remember wayland)
		TODO: some GTK3
		TODO: relative to vs. screen window is at
	*/
	void set_geometry (Pixels w, Pixels h, Pixels left = -1, Pixels top = -1, Pixels right = -1, Pixels bottom = -1, Screen relative = -1, int8_t state = -1); 
    auto get_geometry ();

	/*
		Sets/gets window stacking position.
		The behavior should copy set layer from layer shell wayland protocol.
		---
		OK
	*/
	void set_layer(WindowLayer layer);
	WindowLayer get_layer();

	/*
		Sets/gets window focus policy.
		Came from layer shell protocol.
		---
		TODO: another argument that will select element that can be focused while the window cant
		TODO: in enum definition
		TODO: complete GTK3
	*/
	void set_input_mode_keyboard (WindowInputMode mode);
	WindowInputMode get_input_mode_keyboard ();

	/*
		Makes the window click through.
		---
		TODO: another argument that will select area that is click through
		TODO: better name because this applyes to touch input too
	*/
	void set_input_mode_mouse (WindowInputMode mode);
	WindowInputMode get_input_mode_mouse ();
	
	/*
		Prevent other windows from overlaping.
		Not availible with all window types.
		`zone` value may be: `>0` pixels, `0` dont overlap windows with exclusive zone, `-1` overlap/underlap all windows including those with exclusive zone, `-2` automatic pixels (not returnable since its useless - piexels are returned instead).
		---
		TODO: GTK3 x11
	*/
	void set_exclusive_zone (int16_t zone=-2);
	int16_t get_exclusive_zone ();
	
	/*
		Get whether the window is activated or activate it.
		---
		OK
	*/
	void set_focused (bool activated);
	bool get_focused ();  

	
	/// --- VERY UNSTABLE FEATURES BELOW ---
	
	void start_move_drag ();
	
	void start_resize_drag ();
	
	// TODO: for compositors like wayfire there may be more start_* (like rotate, scale, opacity ctl, ...) - THIS SHOWS WHY CSD IS BAD IDEA
	
	// file vs raw? - same issue with 'content'
	// solution to app vs. window icon - first set is app, then windows (unchangable) +enable hacky animations /window that is the app icon (can i hide it?) -- whats the default behaviour? DO A RESEARCH
	// why would wayland need to get icon by name from icon set (upgaded by app installs/3rd party made) - isnt this the same as app theme so shouldnt the gui tollkits handle that?
	// void set_icon (...) {...} // TODO: big issue because there is "app icon" and "window icon" -> <https://gitlab.freedesktop.org/wayland/wayland-protocols/-/issues/52>  // should it use the html tab icon?
	// ? get_icon ();
	
	
	// void set_geometry_limits (...){...}  // TODO: size limits ~ w, h, ratio + fixed size will have automatically reduced decorations  + position limits  -> <https://docs.gtk.org/gtk3/method.Window.set_geometry_hints.html>
	
	
	// TODO: set_input_mode_mouse <https://stackoverflow.com/questions/11914785/how-to-click-through-gtk-window>
	
	
	void close (); // TODO: close vs. destroy
	
	
	std::function<void()> on_closing = [this](){
		#if defined (HUI_GTK3_DEBUG)
			std::cout<<"\n [HUI::DEBUG] closing window and exiting ";
		#endif
		close();
		HUI::exit();
		};
	
	std::function<void()> on_geometry_changed = [](){}; // TODO: no triggers, 
	
	std::function<void()> on_activated_deactivated = [](){}; // TODO: no triggers, request_activation vs. set_input_keyboard
	
	
	// TODO:  calls: move/resize drag, close=destroy
	// TODO:  callbacks: changed_geometry, activate/deactivate, close_req/window_menu(~titlebar_buttons), drag/drop (NO - will be handled in js), cut/copy/paste (NO - will be handled in js)
	// TODO: what if some of these belong to WindowBase?
};


#if defined(OLDOLD)
class WindowBase { // makes the content a real window - in the future this will be something like nested compositor = xembed combinator = ...
	
  public:
	WindowBase ();
	~WindowBase ();
	
	void add (WebviewContent* content);
	
	/*
		Makes the window show up for the first time.
		---
		TODO: add hide()
		TODO: consider moving this to window controls OR add set_visible(bool) - so popups and other dialog windows can be reused
	*/
	void show ();
	
	/*
		Returns raw pointer to backend object for window.
		---
		TODO: add something like HUI::backend_name so caller can know the type
	*/
	void* backend_window();
	
	// TODO: implement positioning inside window, window embeding, simple bitmap content -- currently can handle only one element=content (for more, its required to implement positioning within the window)

  private:
	//GtkWidget* window = NULL;
	//QWidget* window;
	void* window = NULL;
	//GtkWidget* content = NULL; // should be some kind of structure
	
	
};
#endif

// cpp thing
class Window : /*public WindowBase,*/ public WindowControls, public WebviewContent  { // WebviewContent can be part of this class only until some kind of positioning inside the window is implemented
  public:
	Window() : /*WindowBase(),*/ WebviewContent(), WindowControls()  { /*WindowBase::add(this);*/ WindowControls::use(this); }
	
};




};//namespace


/*
TODO..

would to_base64 be effective?
geomrtry relative=allowed_non_panel_space would solve the issue with li de bg layer shell
li ai has to use python - easier + tensorflow/huggingface


HOM MUCH SHOULD THIS BE MINIMALISTIC API?
what about common webview code?
(maybe in the future there will be "extensions")

// quality of dev life improvements
#define to_js HUI::Window::to_js
#define WindowGeometry() //???
*/

//#if defined(usecabifromdynlib)
// wrap hui.h
//elif
//#include "../hui_new.cc" 
//#endif

#endif //HUI_HH